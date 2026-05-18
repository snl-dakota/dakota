#ifndef DAKOTA_PARSER_HPP
#define DAKOTA_PARSER_HPP

#include "dakota_ast.hpp"

#include <cstddef>
#include <string>

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

bool validate_document(const Document& doc);
bool analyze_semantics(Document& doc);
bool expand_defaults(Document& doc);

} // namespace dakota

#endif // DAKOTA_PARSER_HPP
