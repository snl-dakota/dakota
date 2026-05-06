// dakota_validator.hpp - Constraint validation for Dakota input files
// Validates parsed AST against constraints extracted from XML

#ifndef DAKOTA_VALIDATOR_HPP
#define DAKOTA_VALIDATOR_HPP

#include "dakota_ast.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <variant>
#include <cmath>
#include <algorithm>
#include <iostream>

// Compile-time debug flag
#ifndef DAKOTA_PARSER_DEBUG
#define DAKOTA_PARSER_DEBUG 0
#endif

namespace dakota {

// Import the global debug flag
extern bool g_enable_debug;

// Debug output macro for validator
#define VAL_DEBUG_OUT(msg) \
    do { \
        if (DAKOTA_PARSER_DEBUG && g_enable_debug) { \
            std::cout << "[VAL] " << msg; \
        } \
    } while(0)

// Validation error
struct ValidationError {
    std::string message;
    std::string location;  // Path to the problematic keyword
    
    std::string to_string() const {
        if (location.empty()) {
            return message;
        }
        return location + ": " + message;
    }
};

// Base validator class
// ConstraintsType must be a struct with static methods:
//   - get_occurrence_constraints()
//   - get_param_constraints()
//   - get_oneof_groups()
template<typename ConstraintsType>
class Validator {
public:
    std::vector<ValidationError> errors;
    
    // Validate entire document
    void validate(const Document& doc) {
        VAL_DEBUG_OUT("Starting constraint validation\n");
        for (const auto& block : doc.blocks) {
            validate_block(block);
        }
        VAL_DEBUG_OUT("Validation complete. Errors found: " << errors.size() << "\n");
    }

    // Validate a single block
    void validate(const Block& block) {
        VAL_DEBUG_OUT("Starting constraint validation for block\n");
        validate_block(block);
        VAL_DEBUG_OUT("Validation complete. Errors found: " << errors.size() << "\n");
    }
    
    // Get all validation errors
    const std::vector<ValidationError>& get_errors() const {
        return errors;
    }
    
    bool has_errors() const {
        return !errors.empty();
    }
    
private:
    // Validate a single block
    void validate_block(const Block& block) {
        std::string block_path = block.name;
        VAL_DEBUG_OUT("Validating block: " << block_path << "\n");
        
        // Check occurrence constraints
        check_occurrence_constraints(block, block_path);
        
        // Check oneOf groups  
        check_oneof_constraints(block, block_path);
        
        // Recursively check children
        for (const auto& [kw_name, kw_list] : block.keywords) {
            for (const auto& kw_node : kw_list) {
                std::string kw_path = block_path + "." + kw_name;
                validate_keyword(kw_node, kw_path);
            }
        }
    }
    
    // Validate a keyword and its children
    void validate_keyword(const std::shared_ptr<KeywordNode>& node, const std::string& path) {
        if (!node) return;
        
        VAL_DEBUG_OUT("  Checking: " << path << "\n");
        
        // Check parameter constraints
        if (node->has_param_values()) {
            VAL_DEBUG_OUT("    Has " << node->param_values.size() << " param values\n");
            check_param_constraints(node, path);
        }
        
        // Check occurrence constraints for children
        std::map<std::string, int> child_counts;
        for (const auto& [child_name, child_list] : node->children) {
            child_counts[child_name] = child_list.size();
        }
        
        if (!child_counts.empty()) {
            VAL_DEBUG_OUT("    Children counts:\n");
            for (const auto& [child_name, count] : child_counts) {
                VAL_DEBUG_OUT("      " << child_name << ": " << count << "\n");
                std::string child_path = path + "." + child_name;
                check_single_occurrence(child_name, count, child_path);
            }
        }
        
        // Check oneOf groups at this level
        check_oneof_at_level(node->children, path);
        
        // Recursively validate children
        for (const auto& [child_name, child_list] : node->children) {
            for (const auto& child : child_list) {
                std::string child_path = path + "." + child_name;
                validate_keyword(child, child_path);
            }
        }
    }
    
    // Check occurrence constraints for a block
    void check_occurrence_constraints(const Block& block, const std::string& path) {
        // Count occurrences of each keyword
        std::map<std::string, int> counts;
        for (const auto& [kw_name, kw_list] : block.keywords) {
            counts[kw_name] = kw_list.size();
        }
        
        VAL_DEBUG_OUT("  Block keyword counts:\n");
        for (const auto& [kw_name, count] : counts) {
            VAL_DEBUG_OUT("    " << kw_name << ": " << count << "\n");
        }
        
        // Check against constraints
        for (const auto& [kw_name, count] : counts) {
            check_single_occurrence(kw_name, count, path + "." + kw_name);
        }
    }
    
    // Check single keyword occurrence
    void check_single_occurrence(const std::string& keyword, int count, const std::string& path) {
        const auto& constraints = ConstraintsType::get_occurrence_constraints();
        auto it = constraints.find(keyword);
        if (it == constraints.end()) {
            VAL_DEBUG_OUT("      No occurrence constraint for: " << keyword << "\n");
            return;  // No constraints for this keyword
        }
        
        const auto& constraint = it->second;
        VAL_DEBUG_OUT("      Constraint for " << keyword << ": min=" << constraint.min_occurs 
                     << ", max=" << constraint.max_occurs << "\n");
        
        // Check minimum
        if (constraint.min_occurs >= 0 && count < constraint.min_occurs) {
            std::ostringstream oss;
            oss << "Keyword '" << keyword << "' must appear at least " 
                << constraint.min_occurs << " time(s), but appears " << count << " time(s)";
            VAL_DEBUG_OUT("      ERROR: " << oss.str() << "\n");
            errors.push_back({oss.str(), path});
        }
        
        // Check maximum
        if (constraint.max_occurs >= 0 && count > constraint.max_occurs) {
            std::ostringstream oss;
            oss << "Keyword '" << keyword << "' may appear at most " 
                << constraint.max_occurs << " time(s), but appears " << count << " time(s)";
            VAL_DEBUG_OUT("      ERROR: " << oss.str() << "\n");
            errors.push_back({oss.str(), path});
        }
    }
    
    // Check oneOf groups at block level
    void check_oneof_constraints(const Block& block, const std::string& path) {
        const auto& groups = ConstraintsType::get_oneof_groups();
        VAL_DEBUG_OUT("  Checking oneOf groups at block level (" << groups.size() << " groups total)\n");
        
        for (const auto& group : groups) {
            // Only check groups at this level (parent path matches)
            if (group.parent_path != block.name) {
                continue;
            }
            
            VAL_DEBUG_OUT("    Checking group '" << group.label << "' at " << group.parent_path << "\n");
            
            // Count how many keywords from this group appear
            std::vector<std::string> found_keywords;
            for (const auto& kw_name : group.keywords) {
                if (block.keywords.count(kw_name) > 0) {
                    found_keywords.push_back(kw_name);
                    VAL_DEBUG_OUT("      Found: " << kw_name << "\n");
                }
            }
            
            if (found_keywords.size() > 1) {
                std::ostringstream oss;
                oss << "oneOf group '" << group.label << "': only one of {";
                bool first = true;
                for (const auto& kw : group.keywords) {
                    if (!first) oss << ", ";
                    oss << kw;
                    first = false;
                }
                oss << "} may appear, but found: {";
                first = true;
                for (const auto& kw : found_keywords) {
                    if (!first) oss << ", ";
                    oss << kw;
                    first = false;
                }
                oss << "}";
                VAL_DEBUG_OUT("      ERROR: " << oss.str() << "\n");
                errors.push_back({oss.str(), path});
            }
        }
    }
    
    // Check oneOf groups at keyword level
    void check_oneof_at_level(const std::map<std::string, std::vector<std::shared_ptr<KeywordNode>>>& children,
                              const std::string& path) {
        const auto& groups = ConstraintsType::get_oneof_groups();
        for (const auto& group : groups) {
            // Only check groups at this level
            if (group.parent_path != path) {
                continue;
            }
            
            VAL_DEBUG_OUT("    Checking group '" << group.label << "' at " << path << "\n");
            
            // Count how many keywords from this group appear
            std::vector<std::string> found_keywords;
            for (const auto& kw_name : group.keywords) {
                if (children.count(kw_name) > 0) {
                    found_keywords.push_back(kw_name);
                    VAL_DEBUG_OUT("      Found: " << kw_name << "\n");
                }
            }
            
            if (found_keywords.size() > 1) {
                std::ostringstream oss;
                oss << "oneOf group '" << group.label << "': only one of {";
                bool first = true;
                for (const auto& kw : group.keywords) {
                    if (!first) oss << ", ";
                    oss << kw;
                    first = false;
                }
                oss << "} may appear, but found: {";
                first = true;
                for (const auto& kw : found_keywords) {
                    if (!first) oss << ", ";
                    oss << kw;
                    first = false;
                }
                oss << "}";
                VAL_DEBUG_OUT("      ERROR: " << oss.str() << "\n");
                errors.push_back({oss.str(), path});
            }
        }
    }
    
    // Check parameter constraints
    void check_param_constraints(const std::shared_ptr<KeywordNode>& node, const std::string& path) {
        const auto& constraints = ConstraintsType::get_param_constraints();
        
        // Try path-based lookup first (for context-aware constraints)
        auto it = constraints.find(path);
        
        // If path lookup fails, try just the keyword name (fallback for backwards compatibility)
        if (it == constraints.end()) {
            it = constraints.find(node->effective_name());
        }
        
        if (it == constraints.end()) {
            VAL_DEBUG_OUT("    No param constraint for: " << node->effective_name() << " (path: " << path << ")\n");
            return;  // No constraints for this keyword
        }
        
        const std::string& constraint = it->second;
        VAL_DEBUG_OUT("    Param constraint for " << node->effective_name() << " (path: " << path << "): " << constraint << "\n");
        
        // Check each parameter value
        for (size_t i = 0; i < node->param_values.size(); ++i) {
            check_single_param_constraint(node->param_values[i], constraint, path, i);
        }
    }
    
    // Check a single parameter value against a constraint
    void check_single_param_constraint(const Value& value, const std::string& constraint,
                                      const std::string& path, size_t index) {
        // Handle numeric constraints like ">= 0", "> 0", ">= 1.0"
        if (constraint.find(">=") != std::string::npos || 
            constraint.find(">") != std::string::npos ||
            constraint.find("<=") != std::string::npos ||
            constraint.find("<") != std::string::npos) {
            check_numeric_constraint(value, constraint, path, index);
        }
        // Handle LEN constraints (checked later in a separate pass)
        else if (constraint.find("LEN") != std::string::npos) {
            // TODO: LEN constraints require cross-reference checking
            // For now, skip these
            VAL_DEBUG_OUT("    Skipping LEN constraint (not yet implemented)\n");
        }
    }
    
    // Check numeric constraints
    void check_numeric_constraint(const Value& value, const std::string& constraint,
                                 const std::string& path, size_t index) {
        double num_value;
        bool is_numeric = false;
        
        // Extract numeric value
        if (std::holds_alternative<int64_t>(value)) {
            num_value = static_cast<double>(std::get<int64_t>(value));
            is_numeric = true;
            VAL_DEBUG_OUT("      Value[" << index << "]: " << num_value << " (int)\n");
        } else if (std::holds_alternative<double>(value)) {
            num_value = std::get<double>(value);
            is_numeric = true;
            VAL_DEBUG_OUT("      Value[" << index << "]: " << num_value << " (double)\n");
        } else {
            VAL_DEBUG_OUT("      Value[" << index << "]: non-numeric, skipping constraint check\n");
        }
        
        if (!is_numeric) {
            return;  // Can't check non-numeric values
        }
        
        // Parse constraint
        std::string op;
        double bound;
        
        if (constraint.find(">=") != std::string::npos) {
            op = ">=";
            size_t pos = constraint.find(">=");
            bound = std::stod(constraint.substr(pos + 2));
        } else if (constraint.find(">") != std::string::npos) {
            op = ">";
            size_t pos = constraint.find(">");
            bound = std::stod(constraint.substr(pos + 1));
        } else if (constraint.find("<=") != std::string::npos) {
            op = "<=";
            size_t pos = constraint.find("<=");
            bound = std::stod(constraint.substr(pos + 2));
        } else if (constraint.find("<") != std::string::npos) {
            op = "<";
            size_t pos = constraint.find("<");
            bound = std::stod(constraint.substr(pos + 1));
        } else {
            return;  // Unknown constraint format
        }
        
        VAL_DEBUG_OUT("      Checking: " << num_value << " " << op << " " << bound << "\n");
        
        // Check constraint
        bool valid = false;
        if (op == ">=") {
            valid = num_value >= bound;
        } else if (op == ">") {
            valid = num_value > bound;
        } else if (op == "<=") {
            valid = num_value <= bound;
        } else if (op == "<") {
            valid = num_value < bound;
        }
        
        if (!valid) {
            std::ostringstream oss;
            oss << "Parameter " << index << " value " << num_value 
                << " violates constraint: must be " << constraint;
            VAL_DEBUG_OUT("      ERROR: " << oss.str() << "\n");
            errors.push_back({oss.str(), path});
        } else {
            VAL_DEBUG_OUT("      OK\n");
        }
    }
};

} // namespace dakota

#endif // DAKOTA_VALIDATOR_HPP
