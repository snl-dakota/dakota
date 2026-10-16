// dakota_json_input.hpp — JSON input file loading for Dakota
//
// load_json_file() reads a JSON input file, accepting the non-standard
// Infinity/-Infinity/NaN literals that Python's json.dumps() writes.  The
// document is then checked and converted to the parser AST (json_to_ast.hpp)
// and follows the freeform pipeline.

#ifndef DAKOTA_JSON_INPUT_HPP
#define DAKOTA_JSON_INPUT_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace dakota {

using json = nlohmann::json;

// ============================================================================
// File loading
// ============================================================================

// Python's json.dumps() can produce "Infinity", "-Infinity" and "NaN", which
// are not valid JSON.  Before parsing they are replaced with sentinel strings
// (restored to infinities by restore_inf_sentinels) or null.

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

} // namespace dakota

#endif // DAKOTA_JSON_INPUT_HPP
