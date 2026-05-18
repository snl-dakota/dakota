#ifndef DAKOTA_AST_HPP
#define DAKOTA_AST_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <cstdint>

namespace dakota {

// Forward declarations
struct KeywordNode;

// Value types that can be stored (includes bool for boolean flags)
using Value = std::variant<std::string, int64_t, double, bool>;

// A keyword can have:
// 1. A parameter value (from <param>)
// 2. Nested child keywords
// 3. Both a parameter value and nested children
struct KeywordNode {
    std::string name;                              // Keyword name (as parsed, may be truncated or alias)
    std::vector<Value> param_values;               // Values from <param> element
    std::vector<std::string> original_value_strings; // Original string representation of values (for format preservation)
    std::map<std::string, std::vector<std::shared_ptr<KeywordNode>>> children;  // Nested keywords
    bool is_flag = false;                          // True if keyword has no param or children
    
    // Truncation support
    bool unresolved = false;                       // True if this is a truncated/unknown keyword
    std::string resolved_name;                     // Full keyword name after resolution (empty if not resolved)
    
    // Alias resolution support
    bool was_alias = false;                        // True if this keyword was originally parsed as an alias
    std::string original_name;                     // Original name before alias resolution (empty if not an alias)
    std::string canonical_name;                    // Canonical name after alias resolution (for aliases)
    
    // Helper to add a child keyword
    void add_child(const std::string& child_name, std::shared_ptr<KeywordNode> child) {
        children[child_name].push_back(child);
    }
    
    // Helper to check if keyword has children
    bool has_children() const {
        return !children.empty();
    }
    
    // Helper to check if keyword has parameter values
    bool has_param_values() const {
        return !param_values.empty();
    }
    
    // Get the effective name (canonical > resolved > original)
    std::string effective_name() const {
        if (!canonical_name.empty()) return canonical_name;
        if (!resolved_name.empty()) return resolved_name;
        return name;
    }
};

// Top-level block (environment, method, model, variables, interface, responses)
struct Block {
    std::string name;                              // Block name
    std::map<std::string, std::vector<std::shared_ptr<KeywordNode>>> keywords;  // Top-level keywords in block
    
    // Helper to add a keyword to the block
    void add_keyword(const std::string& keyword_name, std::shared_ptr<KeywordNode> keyword) {
        keywords[keyword_name].push_back(keyword);
    }
};

// Top-level document containing all blocks
struct Document {
    std::vector<Block> blocks;
};

// Parser state for tracking current position in AST during parsing
struct ParserState {
    Document doc;
    Block* current_block = nullptr;
    std::shared_ptr<KeywordNode> current_keyword = nullptr;
    std::vector<std::shared_ptr<KeywordNode>> keyword_stack;  // For nested keywords
    bool in_param_value = false;
    std::string current_block_name;
};

} // namespace dakota

#endif // DAKOTA_AST_HPP
