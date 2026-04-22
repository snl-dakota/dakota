#ifndef DAKOTA_OUTER_ACTIONS_HPP
#define DAKOTA_OUTER_ACTIONS_HPP

#include "dakota_outer_grammar.hpp"
#include "dakota_ast.hpp"
#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace dakota {

// Global debug flag (defined in dakota_parser.cpp)
extern bool g_enable_debug;

namespace outer {

// Forward declarations for block parsers
bool parse_environment_block(const std::string& content, Document& doc, bool enable_trace);
bool parse_interface_block(const std::string& content, Document& doc, bool enable_trace);
bool parse_method_block(const std::string& content, Document& doc, bool enable_trace);
bool parse_model_block(const std::string& content, Document& doc, bool enable_trace);
bool parse_responses_block(const std::string& content, Document& doc, bool enable_trace);
bool parse_variables_block(const std::string& content, Document& doc, bool enable_trace);

// Forward declarations for block grammar analyzers
size_t analyze_environment_grammar();
size_t analyze_interface_grammar();
size_t analyze_method_grammar();
size_t analyze_model_grammar();
size_t analyze_responses_grammar();
size_t analyze_variables_grammar();

// ============================================================================
// Outer Parser State
// ============================================================================

struct OuterState {
    std::string current_block_name;
    std::string current_block_content;
    Document* doc = nullptr;
    bool enable_trace = false;
};

// ============================================================================
// Outer Grammar Actions
// ============================================================================

template<typename Rule>
struct action {};

// Capture block names
template<> struct action<environment_kw> {
    template<typename ActionInput>
    static void apply([[maybe_unused]] const ActionInput& in, OuterState& state) {
        state.current_block_name = "environment";
    }
};

template<> struct action<interface_kw> {
    template<typename ActionInput>
    static void apply([[maybe_unused]] const ActionInput& in, OuterState& state) {
        state.current_block_name = "interface";
    }
};

template<> struct action<method_kw> {
    template<typename ActionInput>
    static void apply([[maybe_unused]] const ActionInput& in, OuterState& state) {
        state.current_block_name = "method";
    }
};

template<> struct action<model_kw> {
    template<typename ActionInput>
    static void apply([[maybe_unused]] const ActionInput& in, OuterState& state) {
        state.current_block_name = "model";
    }
};

template<> struct action<responses_kw> {
    template<typename ActionInput>
    static void apply([[maybe_unused]] const ActionInput& in, OuterState& state) {
        state.current_block_name = "responses";
    }
};

template<> struct action<variables_kw> {
    template<typename ActionInput>
    static void apply([[maybe_unused]] const ActionInput& in, OuterState& state) {
        state.current_block_name = "variables";
    }
};

// Capture and dispatch block content
template<> struct action<block> {
    template<typename ActionInput>
    static void apply(const ActionInput& in, OuterState& state) {
        std::string full_text = in.string();
        
        // BUG FIX: Find where the keyword actually appears in full_text
        // (in.string() includes opt_separator before the keyword)
        size_t keyword_pos = full_text.find(state.current_block_name);
        if (keyword_pos == std::string::npos) {
            throw std::runtime_error("Block keyword '" + state.current_block_name + 
                                   "' not found in matched text");
        }
        
        // Find where content starts (after block name and optional comma/whitespace)
        size_t content_start = keyword_pos + state.current_block_name.length();
        
        // Skip whitespace and optional comma
        while (content_start < full_text.length() && 
               (std::isspace(full_text[content_start]) || full_text[content_start] == ',')) {
            content_start++;
        }
        
        // Normalize "keyword = bare_identifier" → "keyword\n  bare_identifier"
        // Dakota DSL allows e.g. "allocation_target = variance" as an alternative
        // to "allocation_target\n  variance". The inner grammar expects flag keywords
        // as separate tokens; the "= identifier" form bypasses that.
        // We pre-process here so the inner parser sees the canonical form.
        {
            std::string raw_content = full_text.substr(content_start);
            std::string normalized;
            normalized.reserve(raw_content.size() + 32);
            size_t n = raw_content.size();
            size_t p = 0;
            while (p < n) {
                // Look for pattern: whitespace* '=' whitespace* [a-zA-Z_][a-zA-Z0-9_]*
                // but only when NOT inside a quoted string and NOT followed by another '='
                if (raw_content[p] == '=') {
                    // Check it's not '==' 
                    if (p + 1 < n && raw_content[p + 1] == '=') {
                        normalized += raw_content[p++];
                        continue;
                    }
                    // Scan forward past whitespace
                    size_t q = p + 1;
                    while (q < n && (raw_content[q] == ' ' || raw_content[q] == '\t')) ++q;
                    // Check if next non-whitespace is a bare identifier (alpha or _)
                    if (q < n && (std::isalpha((unsigned char)raw_content[q]) || raw_content[q] == '_')) {
                        // Scan the identifier
                        size_t id_start = q;
                        while (q < n && (std::isalnum((unsigned char)raw_content[q]) || raw_content[q] == '_')) ++q;
                        // Check it's not followed by '=' (would be "key = other_key = val" form)
                        size_t after = q;
                        while (after < n && (raw_content[after] == ' ' || raw_content[after] == '\t')) ++after;
                        bool next_is_eq = (after < n && raw_content[after] == '=' && 
                                           (after + 1 >= n || raw_content[after + 1] != '='));
                        // Don't rewrite infinity literals — they are values, not keyword names.
                        std::string id_str = raw_content.substr(id_start, q - id_start);
                        std::string id_lower = id_str;
                        std::transform(id_lower.begin(), id_lower.end(), id_lower.begin(), ::tolower);
                        bool is_inf_literal = (id_lower == "inf" || id_lower == "infinity"
                                               || id_lower == "nan");
                        if (!next_is_eq && !is_inf_literal) {
                            // Replace "= identifier" with "\n  identifier"
                            normalized += "\n  ";
                            normalized += id_str;
                            p = q;
                            continue;
                        }
                    }
                }
                // Skip # comments — append everything from '#' to end-of-line unchanged.
                // Without this, "= identifier" inside a comment would be converted.
                if (raw_content[p] == '#') {
                    while (p < n && raw_content[p] != '\n') normalized += raw_content[p++];
                    continue;
                }
                // Handle quoted strings — pass through unchanged.
                // If a quote immediately follows a keyword (no space), insert one.
                // e.g. "descriptors'x1'" → "descriptors 'x1'"
                if (raw_content[p] == '\'' || raw_content[p] == '"') {
                    if (!normalized.empty()) {
                        char last = normalized.back();
                        if (std::isalnum((unsigned char)last) || last == '_') {
                            normalized += ' ';
                        }
                    }
                    char quote = raw_content[p];
                    normalized += raw_content[p++];
                    while (p < n && raw_content[p] != quote) normalized += raw_content[p++];
                    if (p < n) normalized += raw_content[p++];  // closing quote
                    continue;
                }
                normalized += raw_content[p++];
            }
            state.current_block_content = normalized;
        }
        
        // Debug: Print content being passed to inner grammar with line numbers
        #ifdef DAKOTA_PARSER_DEBUG
        if (g_enable_debug) {
            std::cout << "\n=== Outer->Inner Grammar Handoff for " << state.current_block_name << " block ===\n";
            std::cout << "Content being passed to inner grammar (" << state.current_block_content.length() << " chars):\n";
            std::cout << "----------------------------------------\n";
            
            // Print with line numbers
            std::istringstream iss(state.current_block_content);
            std::string line;
            int line_num = 1;
            while (std::getline(iss, line)) {
                std::cout << std::setw(4) << line_num << " | " << line << "\n";
                line_num++;
            }
            std::cout << "----------------------------------------\n";
            std::cout << "=== End of content for " << state.current_block_name << " block ===\n\n";
        }
        #endif
        
        // Dispatch to appropriate block parser
        bool success = false;
        
        if (state.current_block_name == "environment") {
            success = parse_environment_block(state.current_block_content, *state.doc, state.enable_trace);
        }
        else if (state.current_block_name == "interface") {
            success = parse_interface_block(state.current_block_content, *state.doc, state.enable_trace);
        }
        else if (state.current_block_name == "method") {
            success = parse_method_block(state.current_block_content, *state.doc, state.enable_trace);
        }
        else if (state.current_block_name == "model") {
            success = parse_model_block(state.current_block_content, *state.doc, state.enable_trace);
        }
        else if (state.current_block_name == "responses") {
            success = parse_responses_block(state.current_block_content, *state.doc, state.enable_trace);
        }
        else if (state.current_block_name == "variables") {
            success = parse_variables_block(state.current_block_content, *state.doc, state.enable_trace);
        }
        else {
            throw std::runtime_error("Unknown block type: " + state.current_block_name);
        }
        
        if (!success) {
            throw std::runtime_error("Failed to parse " + state.current_block_name + " block");
        }
    }
};

} // namespace outer
} // namespace dakota

#endif // DAKOTA_OUTER_ACTIONS_HPP
