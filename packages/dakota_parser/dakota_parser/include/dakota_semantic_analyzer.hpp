// dakota_semantic_analyzer.hpp - Semantic analysis for Dakota AST
// Resolves aliases based on parent context

#ifndef DAKOTA_SEMANTIC_ANALYZER_HPP
#define DAKOTA_SEMANTIC_ANALYZER_HPP

#include "dakota_ast.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>

// Compile-time debug flag
#ifndef DAKOTA_PARSER_DEBUG
#define DAKOTA_PARSER_DEBUG 0
#endif

namespace dakota {

// Import the global debug flag
extern bool g_enable_debug;

// Debug output macro for semantic analyzer
#define SEM_DEBUG_OUT(msg) \
    do { \
        if (DAKOTA_PARSER_DEBUG && g_enable_debug) { \
            std::cout << "[SEM] " << msg; \
        } \
    } while(0)

// Semantic analysis error
struct SemanticError {
    std::string message;
    std::string location;  // Path to the problematic keyword
    
    std::string to_string() const {
        if (location.empty()) {
            return message;
        }
        return location + ": " + message;
    }
};

// Semantic analyzer
// SemanticMetadata must be a struct with get_alias_map() static method
template<typename SemanticMetadata>
class SemanticAnalyzer {
public:
    std::vector<SemanticError> errors;
    int aliases_resolved = 0;
    
    // Analyze entire document
    void analyze(Document& doc) {
        SEM_DEBUG_OUT("Starting semantic analysis\n");
        for (auto& block : doc.blocks) {
            analyze_block(block);
        }
        SEM_DEBUG_OUT("Semantic analysis complete. Aliases resolved: " << aliases_resolved << "\n");
    }

    // Analyze a single block
    void analyze(Block& block) {
        SEM_DEBUG_OUT("Starting semantic analysis for block\n");
        analyze_block(block);
        SEM_DEBUG_OUT("Semantic analysis complete. Aliases resolved: " << aliases_resolved << "\n");
    }
    
    // Get analysis statistics
    int get_aliases_resolved() const {
        return aliases_resolved;
    }
    
    const std::vector<SemanticError>& get_errors() const {
        return errors;
    }
    
    bool has_errors() const {
        return !errors.empty();
    }
    
private:
    // Analyze a single block
    void analyze_block(Block& block) {
        std::string block_path = block.name;
        SEM_DEBUG_OUT("Analyzing block: " << block_path << "\n");
        
        // Process top-level keywords
        for (auto& [kw_name, kw_list] : block.keywords) {
            SEM_DEBUG_OUT("  Processing keyword: " << kw_name << " (" << kw_list.size() << " occurrences)\n");
            for (auto& kw_node : kw_list) {
                std::string kw_path = block_path + "." + kw_name;
                analyze_keyword(kw_node, block_path, 2);
            }
        }
    }
    
    // Analyze a keyword and its children recursively
    void analyze_keyword(std::shared_ptr<KeywordNode>& node, const std::string& parent_path, int indent = 0) {
        if (!node) return;
        
        std::string indent_str(indent, ' ');
        std::string current_path = parent_path + "." + node->name;
        
        SEM_DEBUG_OUT(indent_str << "Keyword: " << node->name);
        if (node->has_param_values()) {
            SEM_DEBUG_OUT(" (values: " << node->param_values.size() << ")");
        }
        SEM_DEBUG_OUT("\n");
        
        // Debug: print parameter values
        if (node->has_param_values()) {
            for (size_t i = 0; i < node->param_values.size(); ++i) {
                SEM_DEBUG_OUT(indent_str << "  param[" << i << "]: ");
                const auto& val = node->param_values[i];
                if (std::holds_alternative<int64_t>(val)) {
                    SEM_DEBUG_OUT(std::get<int64_t>(val) << " (int)\n");
                } else if (std::holds_alternative<double>(val)) {
                    SEM_DEBUG_OUT(std::get<double>(val) << " (double)\n");
                } else if (std::holds_alternative<std::string>(val)) {
                    SEM_DEBUG_OUT("'" << std::get<std::string>(val) << "' (string)\n");
                }
            }
        }
        
        // Check if this keyword name is actually an alias in this context
        const auto& alias_map = SemanticMetadata::get_alias_map();
        auto key = std::make_pair(parent_path, node->name);
        auto it = alias_map.find(key);
        
        if (it != alias_map.end()) {
            // This is an alias! Resolve it to canonical name
            node->was_alias = true;
            node->original_name = node->name;
            node->canonical_name = it->second;
            aliases_resolved++;
            
            SEM_DEBUG_OUT(indent_str << "  -> ALIAS resolved: " << node->name 
                         << " -> " << node->canonical_name << "\n");
        }

        if (node->unresolved) {
            std::ostringstream oss;
            oss << "Unknown or ambiguous keyword '" << node->name << "'";
            errors.push_back({oss.str(), current_path});
            SEM_DEBUG_OUT(indent_str << "  -> ERROR: " << oss.str() << "\n");
        }
        
        // Recursively analyze children
        // Use canonical name if resolved, otherwise use original name
        std::string child_parent_path = parent_path + "." + node->effective_name();
        for (auto& [child_name, child_list] : node->children) {
            SEM_DEBUG_OUT(indent_str << "  Children '" << child_name << "': " << child_list.size() << "\n");
            for (auto& child : child_list) {
                analyze_keyword(child, child_parent_path, indent + 4);
            }
        }
    }
};

} // namespace dakota

#endif // DAKOTA_SEMANTIC_ANALYZER_HPP
