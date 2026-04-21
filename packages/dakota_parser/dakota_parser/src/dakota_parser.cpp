// dakota_parser.cpp - Parser with custom error messages
// Updated to use PEGTL's template variable error message approach

#include "dakota_ast.hpp"
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
#include <tao/pegtl/contrib/analyze.hpp>
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

void print_parse_error(const std::string& filename, 
                      const tao::pegtl::parse_error& e,
                      const tao::pegtl::file_input<>* input = nullptr)
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
    return tao::pegtl::analyze<outer::grammar>();
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

// Parse Dakota file using staged approach with custom error messages
bool parse_dakota_file(const std::string& filename, Document& doc, bool enable_trace = false) {
    tao::pegtl::file_input input(filename);
    
    outer::OuterState outer_state;
    outer_state.doc = &doc;
    outer_state.enable_trace = enable_trace;
    
    try {
        if (enable_trace) {
            std::cout << "\n=== Tracing OUTER grammar ===\n";
            return tao::pegtl::standard_trace<outer::grammar, outer::action, error_control>(input, outer_state);
        } else {
            return tao::pegtl::parse<outer::grammar, outer::action, error_control>(input, outer_state);
        }
    } catch (const tao::pegtl::parse_error& e) {
        print_parse_error(filename, e, &input);
        return false;
    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n\n";
        return false;
    }
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

} // namespace dakota

// =============================================================================
// Document Printing Functions
// =============================================================================

// Format a value for Dakota output
std::string format_value(const dakota::Value& val) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
            // Strings in Dakota are quoted
            return "'" + arg + "'";
        } else if constexpr (std::is_same_v<T, int>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, double>) {
            // Format doubles, removing trailing zeros
            std::string result = std::to_string(arg);
            if (result.find('.') != std::string::npos) {
                result.erase(result.find_last_not_of('0') + 1, std::string::npos);
                if (result.back() == '.') {
                    result += '0';
                }
            }
            return result;
        }
        return "";
    }, val);
}

// Print keyword in Dakota format
void print_keyword_dakota(const dakota::KeywordNode& kw, int indent = 0) {
    std::string ind(indent * 2, ' ');
    
    // Use resolved name (not truncated version)
    std::cout << ind << kw.effective_name();
    
    // Print parameter values if present
    if (kw.has_param_values()) {
        std::cout << " = ";
        for (size_t i = 0; i < kw.param_values.size(); ++i) {
            if (i > 0) std::cout << " ";
            
            // Use original string representation if available (preserves format)
            if (i < kw.original_value_strings.size()) {
                const auto& orig_str = kw.original_value_strings[i];
                
                // For quoted strings, output with quotes intact
                if (orig_str.size() >= 2 && 
                    ((orig_str.front() == '\'' && orig_str.back() == '\'') ||
                     (orig_str.front() == '"' && orig_str.back() == '"'))) {
                    std::cout << orig_str;
                } else {
                    // For numbers and identifiers, output as-is (preserves exponential notation)
                    std::cout << orig_str;
                }
            } else {
                // Fallback to formatted value if original string not available
                std::cout << format_value(kw.param_values[i]);
            }
        }
    }
    
    std::cout << "\n";
    
    // Print children recursively
    if (kw.has_children()) {
        for (const auto& [child_name, child_list] : kw.children) {
            for (const auto& child : child_list) {
                print_keyword_dakota(*child, indent + 1);
            }
        }
    }
}

// Print entire document in Dakota format
void print_document_dakota(const dakota::Document& doc) {
    bool first_block = true;
    
    for (const auto& block : doc.blocks) {
        // Add blank line between blocks (except before first)
        if (!first_block) {
            std::cout << "\n";
        }
        first_block = false;
        
        // Print block name
        std::cout << block.name << "\n";
        
        // Print all keywords in this block
        for (const auto& [kw_name, kw_list] : block.keywords) {
            for (const auto& kw : kw_list) {
                print_keyword_dakota(*kw, 1);
            }
        }
    }
}

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [options] <dakota_input_file>\n\n";
    std::cout << "Options:\n";
    std::cout << "  --trace              Enable grammar tracing\n";
    std::cout << "  --analyze            Analyze grammar for issues\n";
    std::cout << "  --no-validate        Skip constraint validation\n";
    std::cout << "  --no-semantic        Skip semantic analysis\n";
    std::cout << "  --no-expand          Skip default expansion\n";
    std::cout << "  --json               Output in JSON format\n";
    std::cout << "  --debug              Enable debug output\n";
    std::cout << "  -h, --help           Show this help message\n";
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Parse command line options
    bool enable_trace = false;
    bool do_analyze = false;
    bool do_validate = true;
    bool do_semantic = true;
    bool do_expand = true;
    bool output_json = false;
    std::string filename;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--trace") {
            enable_trace = true;
        } else if (arg == "--analyze") {
            do_analyze = true;
        } else if (arg == "--no-validate") {
            do_validate = false;
        } else if (arg == "--no-semantic") {
            do_semantic = false;
        } else if (arg == "--no-expand") {
            do_expand = false;
        } else if (arg == "--json") {
            output_json = true;
        } else if (arg == "--debug") {
            dakota::g_enable_debug = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        } else {
            filename = arg;
        }
    }
    
    if (filename.empty()) {
        std::cerr << "Error: No input file specified\n\n";
        print_usage(argv[0]);
        return 1;
    }
    
    // Check if file exists
    if (!std::filesystem::exists(filename)) {
        std::cerr << "Error: File not found: " << filename << "\n";
        return 1;
    }
    
    // Analyze grammars if requested
    if (do_analyze) {
        std::cout << "\n=== Grammar Analysis ===\n";
        size_t issues = dakota::analyze_all_grammars();
        std::cout << "\nTotal issues: " << issues << "\n\n";
        if (issues > 0) {
            return 1;
        }
    }
    
    // Parse the file
    if (dakota::g_enable_debug) {
        std::cout << "Parsing: " << filename << "\n";
    }
    
    dakota::Document doc;
    
    if (!dakota::parse_dakota_file(filename, doc, enable_trace)) {
        return 1;
    }
    
    if (dakota::g_enable_debug) {
        std::cout << "âœ“ Parse successful\n";
    }
    
    // Validate constraints
    if (do_validate) {
        if (dakota::g_enable_debug) {
            std::cout << "\nValidating constraints...\n";
        }
        if (!dakota::validate_document(doc)) {
            std::cerr << "âœ— Validation failed\n";
            return 1;
        }
        if (dakota::g_enable_debug) {
            std::cout << "âœ“ Validation successful\n";
        }
    }
    
    // Semantic analysis
    if (do_semantic) {
        if (dakota::g_enable_debug) {
            std::cout << "\nPerforming semantic analysis...\n";
        }
        if (!dakota::analyze_semantics(doc)) {
            std::cerr << "âœ— Semantic analysis failed\n";
            return 1;
        }
        if (dakota::g_enable_debug) {
            std::cout << "âœ“ Semantic analysis successful\n";
        }
    }
    
    // Expand defaults
    if (do_expand) {
        if (dakota::g_enable_debug) {
            std::cout << "\nExpanding defaults...\n";
        }
        if (!dakota::expand_defaults(doc)) {
            std::cerr << "âœ— Default expansion failed\n";
            return 1;
        }
        if (dakota::g_enable_debug) {
            std::cout << "âœ“ Default expansion successful\n";
        }
    }
    
    // Convert to JSON if requested
    dakota::json json_output;
    if (output_json) {
        if (dakota::g_enable_debug) {
            std::cout << "\nConverting AST to JSON...\n";
        }
        json_output = dakota::ast_to_json(doc);
        if (dakota::g_enable_debug) {
            std::cout << "âœ“ Conversion successful\n";
        }
    }
    
    // Run JSON-level validation (cross-field validators, computed fields)
    if (output_json) {
        dakota::validation_metadata::g_validation_debug = dakota::g_enable_debug;
        std::vector<std::string> val_errors;
        int n_errors = dakota::validation_metadata::validate_json_document(json_output, val_errors);
        if (n_errors > 0) {
            for (const auto& e : val_errors) {
                std::cerr << "Error: " << e << "\n";
            }
            return 1;
        }
    }
    
    // Print summary if in debug mode
    if (dakota::g_enable_debug) {
        std::cout << "\n=== Summary ===\n";
        std::cout << "Blocks parsed: " << doc.blocks.size() << "\n";
        for (const auto& block : doc.blocks) {
            std::cout << "  - " << block.name << " (" << block.keywords.size() << " keywords)\n";
        }
        std::cout << "\nâœ“ All checks passed!\n\n";
    }
    
    // Output results
    if (output_json) {
        // Output JSON
        std::cout << dakota::json_to_string(json_output, 2) << "\n";
    } else {
        // Print the validated Dakota file (fully expanded, no truncations)
        print_document_dakota(doc);
    }
    
    return 0;
}
