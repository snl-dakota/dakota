#ifndef DAKOTA_PARSER_HPP
#define DAKOTA_PARSER_HPP

#include "dakota_ast.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace dakota {

size_t analyze_outer_grammar();
size_t analyze_all_grammars();

bool parse_dakota_file(const std::string& filename,
                       Document& doc,
                       bool enable_trace = false);
bool parse_dakota_string(const std::string& input_text,
                         const std::string& source_name,
                         Document& doc,
                         bool enable_trace = false);

// These print their errors to stderr; the overloads taking `errors` append
// them there instead.
bool validate_document(const Document& doc);
bool validate_document(const Document& doc, std::vector<std::string>& errors);
bool analyze_semantics(Document& doc);
bool analyze_semantics(Document& doc, std::vector<std::string>& errors);
// api_mode=true relaxes required-ness checks for block-pointer fields.
bool expand_defaults(Document& doc, bool api_mode = false);
bool expand_defaults(Document& doc, std::vector<std::string>& errors,
                     bool api_mode = false);

} // namespace dakota

#endif // DAKOTA_PARSER_HPP
