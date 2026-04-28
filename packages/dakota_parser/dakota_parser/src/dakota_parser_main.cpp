#include "dakota_parser.hpp"

#include "ast_to_json.hpp"
#include "dakota_default_expander.hpp"
#include "dakota_input_reader.hpp"
#include "dakota_json_input.hpp"
#include "dakota_semantic_analyzer.hpp"
#include "dakota_validation_metadata.hpp"
#include "dakota_validator.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <variant>

namespace {

std::string format_value(const dakota::Value& val)
{
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return "'" + arg + "'";
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, double>) {
            return dakota::dsl_float_repr(arg);
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
        }
        return "";
    }, val);
}

void print_keyword_dakota(const dakota::KeywordNode& kw, int indent = 0)
{
    std::string ind(indent * 2, ' ');

    std::cout << ind << kw.effective_name();

    bool all_bool_true = kw.has_param_values();
    for (const auto& v : kw.param_values) {
        if (!std::holds_alternative<bool>(v) || !std::get<bool>(v)) {
            all_bool_true = false;
            break;
        }
    }
    if (kw.has_param_values() && !all_bool_true) {
        std::cout << " = ";
        for (size_t i = 0; i < kw.param_values.size(); ++i) {
            if (i > 0) {
                std::cout << " ";
            }

            bool is_quoted = false;
            if (i < kw.original_value_strings.size()) {
                const auto& orig_str = kw.original_value_strings[i];
                if (orig_str.size() >= 2 &&
                    ((orig_str.front() == '\'' && orig_str.back() == '\'') ||
                     (orig_str.front() == '"'  && orig_str.back() == '"'))) {
                    std::string inner = orig_str.substr(1, orig_str.size() - 2);
                    std::cout << '\'' << inner << '\'';
                    is_quoted = true;
                }
            }
            if (!is_quoted) {
                std::cout << format_value(kw.param_values[i]);
            }
        }
    }

    std::cout << "\n";

    if (kw.has_children()) {
        for (const auto& [child_name, child_list] : kw.children) {
            for (const auto& child : child_list) {
                print_keyword_dakota(*child, indent + 1);
            }
        }
    }
}

void print_document_dakota(const dakota::Document& doc)
{
    bool first_block = true;

    for (const auto& block : doc.blocks) {
        if (!first_block) {
            std::cout << "\n";
        }
        first_block = false;

        std::cout << block.name << "\n";

        for (const auto& [kw_name, kw_list] : block.keywords) {
            (void)kw_name;
            for (const auto& kw : kw_list) {
                print_keyword_dakota(*kw, 1);
            }
        }
    }
}

void print_usage(const char* prog)
{
    std::cout << "Usage: " << prog << " [options] <input_file>\n\n";
    std::cout << "Input mode (auto-detected from .json extension by default):\n";
    std::cout << "  --json-input         Force JSON input mode (e.g. file lacks .json extension)\n";
    std::cout << "  --dsl-input          Force DSL input mode (overrides .json extension)\n";
    std::cout << "  --schema <path>      Deprecated; JSON input uses embedded schema metadata\n";
    std::cout << "\nDSL-mode options:\n";
    std::cout << "  --trace              Enable grammar tracing\n";
    std::cout << "  --analyze            Analyze grammar for issues\n";
    std::cout << "  --no-semantic        Skip semantic analysis\n";
    std::cout << "  --json               Output in JSON format\n";
    std::cout << "\nCommon options (both modes):\n";
    std::cout << "  --no-validate        Skip constraint/cross-field validation\n";
    std::cout << "  --no-expand          Skip default expansion\n";
    std::cout << "  --debug              Enable debug output\n";
    std::cout << "  -h, --help           Show this help message\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << prog << " study.in --json           # DSL input -> JSON output\n";
    std::cout << "  " << prog << " study.json                # JSON input (auto-detected)\n";
    std::cout << "  " << prog << " study.txt --json-input    # JSON input (forced)\n";
    std::cout << "\n";
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    bool enable_trace  = false;
    bool do_analyze    = false;
    bool do_validate   = true;
    bool do_semantic   = true;
    bool do_expand     = true;
    bool output_json   = false;
    bool force_json_in = false;
    bool force_dsl_in  = false;
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
        } else if (arg == "--json-input") {
            force_json_in = true;
        } else if (arg == "--dsl-input") {
            force_dsl_in = true;
        } else if (arg == "--schema") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --schema requires a path argument\n";
                return 1;
            }
            ++i;
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

    if (!std::filesystem::exists(filename)) {
        std::cerr << "Error: File not found: " << filename << "\n";
        return 1;
    }

    if (force_json_in && force_dsl_in) {
        std::cerr << "Error: --json-input and --dsl-input are mutually exclusive\n";
        return 1;
    }

    bool json_input_mode;
    if (force_json_in) {
        json_input_mode = true;
    } else if (force_dsl_in) {
        json_input_mode = false;
    } else {
        const auto dot = filename.rfind('.');
        const std::string ext = (dot != std::string::npos) ? filename.substr(dot) : "";
        json_input_mode = (ext == ".json");
    }

    if (json_input_mode) {
        if (dakota::g_enable_debug) {
            std::cout << "Input mode: JSON\n";
            std::cout << "Schema:     embedded\n";
            std::cout << "File:       " << filename << "\n";
        }

        dakota::json doc;
        {
            std::vector<std::string> load_errors;
            if (!dakota::load_json_file(filename, doc, load_errors)) {
                for (const auto& e : load_errors) {
                    std::cerr << "Error: " << e << "\n";
                }
                return 1;
            }
        }
        if (dakota::g_enable_debug) {
            std::cout << "\u2713 JSON parse successful\n";
        }

        if (do_expand) {
            if (dakota::g_enable_debug) {
                std::cout << "\nExpanding JSON defaults...\n";
            }
            std::vector<std::string> exp_errors;
            if (!dakota::expand_json_defaults(doc, exp_errors, dakota::g_enable_debug)) {
                for (const auto& e : exp_errors) {
                    std::cerr << "Error: " << e << "\n";
                }
                return 1;
            }
            if (dakota::g_enable_debug) {
                std::cout << "\u2713 Default expansion successful\n";
            }
        }

        if (do_validate) {
            if (dakota::g_enable_debug) {
                std::cout << "\nValidating JSON document...\n";
            }
            dakota::validation_metadata::g_validation_debug = dakota::g_enable_debug;
            std::vector<std::string> val_errors;
            const int n = dakota::validation_metadata::validate_json_document(doc, val_errors);
            if (n > 0) {
                for (const auto& e : val_errors) {
                    std::cerr << "Error: " << e << "\n";
                }
                return 1;
            }
            if (dakota::g_enable_debug) {
                std::cout << "\u2713 Validation successful\n";
            }
        }

        if (dakota::g_enable_debug) {
            std::cout << "\n\u2713 All checks passed!\n\n";
        }

        std::cout << dakota::json_to_string(doc, 2) << "\n";
        return 0;
    }

    if (do_analyze) {
        std::cout << "\n=== Grammar Analysis ===\n";
        const size_t issues = dakota::analyze_all_grammars();
        std::cout << "\nTotal issues: " << issues << "\n\n";
        if (issues > 0) {
            return 1;
        }
    }

    if (dakota::g_enable_debug) {
        std::cout << "Input mode: DSL\n";
        std::cout << "Parsing: " << filename << "\n";
    }

    dakota::Document doc;
    if (!dakota::parse_dakota_file(filename, doc, enable_trace)) {
        return 1;
    }

    if (dakota::g_enable_debug) {
        std::cout << "\u2713 Parse successful\n";
    }

    if (do_validate) {
        if (dakota::g_enable_debug) {
            std::cout << "\nValidating constraints...\n";
        }
        if (!dakota::validate_document(doc)) {
            std::cerr << "\u2717 Validation failed\n";
            return 1;
        }
        if (dakota::g_enable_debug) {
            std::cout << "\u2713 Validation successful\n";
        }
    }

    if (do_semantic) {
        if (dakota::g_enable_debug) {
            std::cout << "\nPerforming semantic analysis...\n";
        }
        if (!dakota::analyze_semantics(doc)) {
            std::cerr << "\u2717 Semantic analysis failed\n";
            return 1;
        }
        if (dakota::g_enable_debug) {
            std::cout << "\u2713 Semantic analysis successful\n";
        }
    }

    if (do_expand) {
        if (dakota::g_enable_debug) {
            std::cout << "\nExpanding defaults...\n";
        }
        if (!dakota::expand_defaults(doc)) {
            std::cerr << "\u2717 Default expansion failed\n";
            return 1;
        }
        if (dakota::g_enable_debug) {
            std::cout << "\u2713 Default expansion successful\n";
        }
    }

    dakota::json json_output;
    if (output_json) {
        if (dakota::g_enable_debug) {
            std::cout << "\nConverting AST to JSON...\n";
        }
        json_output = dakota::ast_to_json(doc);
        if (dakota::g_enable_debug) {
            std::cout << "\u2713 Conversion successful\n";
        }
    }

    if (output_json) {
        dakota::validation_metadata::g_validation_debug = dakota::g_enable_debug;
        std::vector<std::string> val_errors;
        const int n_errors =
            dakota::validation_metadata::validate_json_document(json_output, val_errors);
        if (n_errors > 0) {
            for (const auto& e : val_errors) {
                std::cerr << "Error: " << e << "\n";
            }
            return 1;
        }
    }

    if (dakota::g_enable_debug) {
        std::cout << "\n=== Summary ===\n";
        std::cout << "Blocks parsed: " << doc.blocks.size() << "\n";
        for (const auto& block : doc.blocks) {
            std::cout << "  - " << block.name << " (" << block.keywords.size()
                      << " keywords)\n";
        }
        std::cout << "\n\u2713 All checks passed!\n\n";
    }

    if (output_json) {
        std::cout << dakota::json_to_string(json_output, 2) << "\n";
    } else {
        print_document_dakota(doc);
    }

    return 0;
}
