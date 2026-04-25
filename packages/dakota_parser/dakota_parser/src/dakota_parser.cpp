// dakota_parser.cpp - Parser with custom error messages
// Updated to use PEGTL's template variable error message approach

#include "dakota_parser.hpp"
#include "dakota_input_reader.hpp"
#include "dakota_json_input.hpp"
#include "dakota_grammar_common.hpp"
#include "dakota_outer_grammar.hpp"
#include "dakota_outer_actions.hpp"
#include "dakota_error_messages.hpp"
#include "dakota_validator.hpp"
#include "dakota_semantic_analyzer.hpp"
#include "dakota_default_expander.hpp"
#include "ast_to_json.hpp"
#include "dakota_validation_metadata.hpp"
#include <tao/pegtl.hpp>
#ifndef _MSC_VER
#include <tao/pegtl/contrib/analyze.hpp>
#endif
#include <tao/pegtl/contrib/trace.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>

// Include constraint metadata for all blocks
#include "method/dakota_constraints_method.hpp"
#include "variables/dakota_constraints_variables.hpp"
#include "interface/dakota_constraints_interface.hpp"
#include "responses/dakota_constraints_responses.hpp"
#include "environment/dakota_constraints_environment.hpp"
#include "model/dakota_constraints_model.hpp"

// Include semantic metadata for all blocks
#include "method/dakota_semantic_method.hpp"
#include "variables/dakota_semantic_variables.hpp"
#include "interface/dakota_semantic_interface.hpp"
#include "responses/dakota_semantic_responses.hpp"
#include "environment/dakota_semantic_environment.hpp"
#include "model/dakota_semantic_model.hpp"

namespace dakota {

// Global debug flag definition
bool g_enable_debug = false;

// =============================================================================
// Custom Error Control
// =============================================================================

// Control class that uses our custom error messages
template<typename Rule>
struct error_control : tao::pegtl::normal<Rule>
{
    static const std::string error_message;
    
    template<typename Input, typename... States>
    [[noreturn]] static void raise(const Input& in, States&&...)
    {
        throw tao::pegtl::parse_error(error_message, in);
    }
};

// Specialization for rules that have custom error messages
template<typename Rule>
const std::string error_control<Rule>::error_message = 
    errors::error_message<Rule> ? errors::error_message<Rule> 
                                : "parse error at this location";

// =============================================================================
// Clang-Style Error Printing
// =============================================================================

template <typename Input>
void print_parse_error(const std::string& filename, 
                      const tao::pegtl::parse_error& e,
                      const Input* input = nullptr)
{
    const auto& pos = e.positions().front();
    
    // Print: filename:line:column: error: message
    std::cerr << filename << ":" << pos.line << ":" << pos.column 
              << ": error: " << e.message() << "\n";
    
    // Show source line with caret if input is available
    if (input) {
        try {
            std::string line(input->line_at(pos));
            std::cerr << line << "\n";
            
            // Print caret pointer (column is 1-based)
            if (pos.column > 0) {
                std::cerr << std::string(pos.column - 1, ' ') << "^\n";
            }
        } catch (...) {
            // If we can't get the line, just skip the context
        }
    }
}

// =============================================================================
// Grammar Analysis
// =============================================================================

// Analyze outer grammar for cycles
size_t analyze_outer_grammar() {
#ifdef _MSC_VER
    return 0;
#else
    return tao::pegtl::analyze<outer::grammar>();
#endif
}

// Analyze all grammars (outer + all blocks)
size_t analyze_all_grammars() {
    std::cout << "Analyzing outer grammar...\n";
    size_t total_issues = analyze_outer_grammar();
    if (total_issues > 0) {
        std::cerr << "  Outer grammar has " << total_issues << " issues\n";
    } else {
        std::cout << "  Outer grammar: OK\n";
    }
    
    // Analyze each block grammar
    const char* block_names[] = {"environment", "interface", "method", "model", "responses", "variables"};
    size_t (*analyzers[])() = {
        outer::analyze_environment_grammar,
        outer::analyze_interface_grammar,
        outer::analyze_method_grammar,
        outer::analyze_model_grammar,
        outer::analyze_responses_grammar,
        outer::analyze_variables_grammar
    };
    
    for (size_t i = 0; i < 6; ++i) {
        std::cout << "Analyzing " << block_names[i] << " grammar...\n";
        size_t issues = analyzers[i]();
        if (issues > 0) {
            std::cerr << "  " << block_names[i] << " grammar has " << issues << " issues\n";
            total_issues += issues;
        } else {
            std::cout << "  " << block_names[i] << " grammar: OK\n";
        }
    }
    
    return total_issues;
}

// =============================================================================
// Main Parser Function
// =============================================================================

template <typename Input>
bool parse_dakota_input(Input& input, const std::string& source_name,
                        Document& doc, bool enable_trace = false) {
    outer::OuterState outer_state;
    outer_state.doc = &doc;
    outer_state.enable_trace = enable_trace;

    try {
        if (enable_trace) {
            std::cout << "\n=== Tracing OUTER grammar ===\n";
            return tao::pegtl::standard_trace<outer::grammar, outer::action, error_control>(
                input, outer_state);
        } else {
            return tao::pegtl::parse<outer::grammar, outer::action, error_control>(
                input, outer_state);
        }
    } catch (const tao::pegtl::parse_error& e) {
        print_parse_error(source_name, e, &input);
        return false;
    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n\n";
        return false;
    }
}

// Parse Dakota file using staged approach with custom error messages
bool parse_dakota_file(const std::string& filename, Document& doc, bool enable_trace) {
    tao::pegtl::file_input input(filename);
    return parse_dakota_input(input, filename, doc, enable_trace);
}

bool parse_dakota_string(const std::string& input_text, const std::string& source_name,
                         Document& doc, bool enable_trace) {
    tao::pegtl::memory_input input(input_text, source_name);
    return parse_dakota_input(input, source_name, doc, enable_trace);
}

// =============================================================================
// Document Validation
// =============================================================================

// Validate Dakota document against constraints
bool validate_document(const Document& doc) {
    std::vector<ValidationError> all_errors;
    
    // Validate each block with its corresponding constraints
    for (const auto& block : doc.blocks) {
        if (block.name == "method") {
            Validator<method_constraints> validator;
            validator.validate(doc);
            const auto& errors = validator.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
        else if (block.name == "variables") {
            Validator<variables_constraints> validator;
            validator.validate(doc);
            const auto& errors = validator.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
        else if (block.name == "interface") {
            Validator<interface_constraints> validator;
            validator.validate(doc);
            const auto& errors = validator.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
        else if (block.name == "responses") {
            Validator<responses_constraints> validator;
            validator.validate(doc);
            const auto& errors = validator.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
        else if (block.name == "environment") {
            Validator<environment_constraints> validator;
            validator.validate(doc);
            const auto& errors = validator.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
        else if (block.name == "model") {
            Validator<model_constraints> validator;
            validator.validate(doc);
            const auto& errors = validator.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
    }
    
    // Report validation errors
    if (!all_errors.empty()) {
        std::cerr << "\n=== Validation Errors ===\n";
        for (const auto& error : all_errors) {
            std::cerr << "  " << error.to_string() << "\n";
        }
        std::cerr << "\nTotal validation errors: " << all_errors.size() << "\n";
        return false;
    }
    
    return true;
}

// Perform semantic analysis on Dakota document (resolve aliases)
bool analyze_semantics(Document& doc) {
    int total_resolved = 0;
    std::vector<SemanticError> all_errors;
    
    // Analyze each block with its corresponding semantic metadata
    for (auto& block : doc.blocks) {
        if (block.name == "method") {
            SemanticAnalyzer<method_semantic> analyzer;
            analyzer.analyze(doc);
            total_resolved += analyzer.get_aliases_resolved();
            const auto& errors = analyzer.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
        else if (block.name == "variables") {
            SemanticAnalyzer<variables_semantic> analyzer;
            analyzer.analyze(doc);
            total_resolved += analyzer.get_aliases_resolved();
            const auto& errors = analyzer.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
        else if (block.name == "interface") {
            SemanticAnalyzer<interface_semantic> analyzer;
            analyzer.analyze(doc);
            total_resolved += analyzer.get_aliases_resolved();
            const auto& errors = analyzer.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
        else if (block.name == "responses") {
            SemanticAnalyzer<responses_semantic> analyzer;
            analyzer.analyze(doc);
            total_resolved += analyzer.get_aliases_resolved();
            const auto& errors = analyzer.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
        else if (block.name == "environment") {
            SemanticAnalyzer<environment_semantic> analyzer;
            analyzer.analyze(doc);
            total_resolved += analyzer.get_aliases_resolved();
            const auto& errors = analyzer.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
        else if (block.name == "model") {
            SemanticAnalyzer<model_semantic> analyzer;
            analyzer.analyze(doc);
            total_resolved += analyzer.get_aliases_resolved();
            const auto& errors = analyzer.get_errors();
            all_errors.insert(all_errors.end(), errors.begin(), errors.end());
        }
    }
    
    // Report semantic errors
    if (!all_errors.empty()) {
        std::cerr << "\n=== Semantic Errors ===\n";
        for (const auto& error : all_errors) {
            std::cerr << "  " << error.to_string() << "\n";
        }
        std::cerr << "\nTotal semantic errors: " << all_errors.size() << "\n";
        return false;
    }
    
    if (total_resolved > 0 && g_enable_debug) {
        std::cout << "Resolved " << total_resolved << " keyword aliases\n";
    }
    
    return true;
}

// =============================================================================
// Default Expansion
// =============================================================================

// Expand defaults in Dakota document
bool expand_defaults(Document& doc) {
    std::vector<std::string> errors;
    
    if (!DefaultExpander::expand_document(doc, errors)) {
        std::cerr << "\n=== Default Expansion Errors ===\n";
        for (const auto& error : errors) {
            std::cerr << "  " << error << "\n";
        }
        std::cerr << "\nTotal expansion errors: " << errors.size() << "\n";
        return false;
    }
    
    return true;
}

namespace {

dakota::InputFormat detect_input_format(const std::string& filename,
                                        dakota::InputFormat format) {
    if (format != dakota::InputFormat::Auto) {
        return format;
    }

    auto dot = filename.rfind('.');
    std::string ext = (dot != std::string::npos) ? filename.substr(dot) : "";
    return (ext == ".json") ? dakota::InputFormat::Json
                             : dakota::InputFormat::DakotaFreeform;
}

bool validate_json_document_in_place(json& doc,
                                     std::vector<std::string>& errors,
                                     bool debug) {
    validation_metadata::g_validation_debug = debug;
    std::vector<std::string> val_errors;
    const int n_errors = validation_metadata::validate_json_document(doc, val_errors);
    if (n_errors > 0) {
        errors.insert(errors.end(), val_errors.begin(), val_errors.end());
        return false;
    }
    return true;
}

bool parse_freeform_document_to_json(Document& doc, json& output,
                                     std::vector<std::string>& errors,
                                     const std::string& context,
                                     bool debug) {
    if (!validate_document(doc)) {
        errors.push_back("Constraint validation failed for Dakota freeform input: " + context);
        return false;
    }
    if (!analyze_semantics(doc)) {
        errors.push_back("Semantic analysis failed for Dakota freeform input: " + context);
        return false;
    }
    if (!expand_defaults(doc)) {
        errors.push_back("Default expansion failed for Dakota freeform input: " + context);
        return false;
    }

    output = ast_to_json(doc);
    return validate_json_document_in_place(output, errors, debug);
}

json throw_for_reader_errors(const std::string& context,
                             const std::vector<std::string>& errors) {
    std::string message = context;
    if (!errors.empty()) {
        message += ": " + errors.front();
        for (size_t i = 1; i < errors.size(); ++i) {
            message += "\n" + errors[i];
        }
    }
    throw std::runtime_error(message);
}

} // namespace

bool read_freeform_input_file_to_json(const std::string& filename,
                                      json& output,
                                      std::vector<std::string>& errors,
                                      bool debug)
{
    g_enable_debug = debug;

    Document doc;
    if (!parse_dakota_file(filename, doc, false)) {
        errors.push_back("Failed to parse Dakota freeform input: " + filename);
        return false;
    }
    return parse_freeform_document_to_json(doc, output, errors, filename, debug);
}

json read_freeform_input_file_to_json(const std::string& filename,
                                      bool debug)
{
    json output;
    std::vector<std::string> errors;
    if (!read_freeform_input_file_to_json(filename, output, errors, debug)) {
        return throw_for_reader_errors("Failed to read freeform input file '" + filename + "'",
                                       errors);
    }
    return output;
}

bool read_freeform_input_string_to_json(const std::string& input,
                                        json& output,
                                        std::vector<std::string>& errors,
                                        bool debug)
{
    g_enable_debug = debug;

    Document doc;
    if (!parse_dakota_string(input, "<dakota-input-string>", doc, false)) {
        errors.push_back("Failed to parse Dakota freeform input string.");
        return false;
    }
    return parse_freeform_document_to_json(doc, output, errors,
                                           "<dakota-input-string>", debug);
}

json read_freeform_input_string_to_json(const std::string& input,
                                        bool debug)
{
    json output;
    std::vector<std::string> errors;
    if (!read_freeform_input_string_to_json(input, output, errors, debug)) {
        return throw_for_reader_errors("Failed to read freeform input string", errors);
    }
    return output;
}

bool validate_json_input_to_json(const json& input,
                                 json& output,
                                 std::vector<std::string>& errors,
                                 bool debug)
{
    g_enable_debug = debug;

    output = input;
    if (!expand_json_defaults(output, errors, debug)) {
        return false;
    }
    return validate_json_document_in_place(output, errors, debug);
}

json validate_json_input_to_json(const json& input,
                                 bool debug)
{
    json output;
    std::vector<std::string> errors;
    if (!validate_json_input_to_json(input, output, errors, debug)) {
        return throw_for_reader_errors("Failed to validate JSON input", errors);
    }
    return output;
}

bool read_json_input_file_to_json(const std::string& filename,
                                  json& output,
                                  std::vector<std::string>& errors,
                                  bool debug)
{
    json doc;
    if (!load_json_file(filename, doc, errors)) {
        return false;
    }
    return validate_json_input_to_json(doc, output, errors, debug);
}

json read_json_input_file_to_json(const std::string& filename,
                                  bool debug)
{
    json output;
    std::vector<std::string> errors;
    if (!read_json_input_file_to_json(filename, output, errors, debug)) {
        return throw_for_reader_errors("Failed to read JSON input file '" + filename + "'",
                                       errors);
    }
    return output;
}

bool read_json_input_string_to_json(const std::string& input,
                                    json& output,
                                    std::vector<std::string>& errors,
                                    bool debug)
{
    json doc;
    try {
        doc = json::parse(sanitize_json_for_parse(input));
        restore_inf_sentinels(doc);
    } catch (const json::parse_error& e) {
        errors.push_back(std::string("JSON parse error: ") + e.what());
        return false;
    }
    return validate_json_input_to_json(doc, output, errors, debug);
}

json read_json_input_string_to_json(const std::string& input,
                                    bool debug)
{
    json output;
    std::vector<std::string> errors;
    if (!read_json_input_string_to_json(input, output, errors, debug)) {
        return throw_for_reader_errors("Failed to read JSON input string", errors);
    }
    return output;
}

bool read_input_file_to_json(const std::string& filename,
                             json& output,
                             std::vector<std::string>& errors,
                             InputFormat format,
                             bool debug)
{
    const InputFormat resolved = detect_input_format(filename, format);
    if (resolved == InputFormat::Json) {
        return read_json_input_file_to_json(filename, output, errors, debug);
    }
    return read_freeform_input_file_to_json(filename, output, errors, debug);
}

json read_input_file_to_json(const std::string& filename,
                             InputFormat format,
                             bool debug)
{
    const InputFormat resolved = detect_input_format(filename, format);
    if (resolved == InputFormat::Json) {
        return read_json_input_file_to_json(filename, debug);
    }
    return read_freeform_input_file_to_json(filename, debug);
}

} // namespace dakota
