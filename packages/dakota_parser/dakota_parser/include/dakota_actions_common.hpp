#ifndef DAKOTA_ACTIONS_COMMON_HPP
#define DAKOTA_ACTIONS_COMMON_HPP

#include "dakota_ast.hpp"
#include "dakota_grammar_common.hpp"
#include <cstdlib>
#include <limits>
#include <optional>
#include <iostream>

// Compile-time debug flag
#ifndef DAKOTA_PARSER_DEBUG
#define DAKOTA_PARSER_DEBUG 0
#endif

namespace dakota {
// Global debug flag (set at runtime)
extern bool g_enable_debug;

namespace actions {

// Debug output macro
#define DEBUG_OUT(msg)                                                         \
  do {                                                                         \
    if (DAKOTA_PARSER_DEBUG && g_enable_debug) {                               \
      std::cout << msg;                                                        \
    }                                                                          \
  } while (0)

// ============================================================================
// BASE ACTION TEMPLATE
// ============================================================================

// Base action template - no action by default
template <typename Rule> struct action {};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Helper to create a new keyword node
inline std::shared_ptr<KeywordNode> create_keyword(const std::string &name,
                                                   bool is_flag = false) {
  auto kw = std::make_shared<KeywordNode>();
  kw->name = name;
  kw->is_flag = is_flag;
  return kw;
}

// Helper to add keyword to current context (block or parent keyword)
inline void add_keyword_to_context(std::shared_ptr<KeywordNode> keyword,
                                   ParserState &state) {
  if (state.keyword_stack.empty()) {
    // Add to current block
    if (state.current_block) {
      state.current_block->add_keyword(keyword->name, keyword);
    }
  } else {
    // Add to parent keyword (last on stack)
    state.keyword_stack.back()->add_child(keyword->name, keyword);
  }
}

// Strip surrounding quotes and trim whitespace from a string value.
// Returns the unquoted/trimmed string, or empty optional if not quoted.
inline std::optional<std::string> unquote_and_trim(const std::string &text) {
  if (text.size() >= 2 && ((text.front() == '\'' && text.back() == '\'') ||
                           (text.front() == '"' && text.back() == '"'))) {
    std::string s = text.substr(1, text.size() - 2);
    auto start = s.find_first_not_of(" \t");
    auto end = s.find_last_not_of(" \t");
    if (start != std::string::npos) {
      return s.substr(start, end - start + 1);
    }
    return std::string{};  // All whitespace
  }
  return std::nullopt;  // Not quoted
}

// Parse a text token into a Value (string, int, or double).
// Quoted strings are unquoted+trimmed; bare text is tried as int then double.
inline Value parse_text_to_value(const std::string &text) {
  auto quoted = unquote_and_trim(text);
  if (quoted) {
    return *quoted;
  }
  // Try integer (no decimal, no exponent, fits in int)
  if (text.find('.') == std::string::npos &&
      text.find('e') == std::string::npos &&
      text.find('E') == std::string::npos) {
    char *end = nullptr;
    long long iv = std::strtoll(text.c_str(), &end, 10);
    if (end == text.c_str() + text.size() &&
        iv >= std::numeric_limits<int>::min() &&
        iv <= std::numeric_limits<int>::max()) {
      return static_cast<long int>(iv);
    }
  }
  // Try double
  char *end = nullptr;
  double dv = std::strtod(text.c_str(), &end);
  if (end == text.c_str() + text.size()) {
    return dv;
  }
  // Bare identifier
  return text;
}

// Helper to parse value and add to current keyword
inline void add_value_to_current_keyword(const std::string &text,
                                         ParserState &state) {
  if (!state.current_keyword)
    return;

  state.current_keyword->original_value_strings.push_back(text);
  Value val = parse_text_to_value(text);
  DEBUG_OUT("[DEBUG] Value: " << text << "\n");
  state.current_keyword->param_values.push_back(val);
}

// Helper to add repeated values to current keyword
// Parses "N*value" or "N* value" format and adds N copies of value
inline void add_repeat_value_to_current_keyword(const std::string &repeat_text,
                                                ParserState &state) {
  if (!state.current_keyword)
    return;

  // Find the * separator
  size_t star_pos = repeat_text.find('*');
  if (star_pos == std::string::npos) {
    // Shouldn't happen if grammar is correct, but fall back to single value
    add_value_to_current_keyword(repeat_text, state);
    return;
  }

  // Parse the count (before *)
  std::string count_str = repeat_text.substr(0, star_pos);
  int count = std::atoi(count_str.c_str());
  if (count <= 0) {
    count = 1; // Safety fallback
  }

  // Parse the value (after *), stripping leading whitespace
  std::string value_str = repeat_text.substr(star_pos + 1);
  // Strip leading whitespace from value
  size_t value_start = value_str.find_first_not_of(" \t");
  if (value_start != std::string::npos) {
    value_str = value_str.substr(value_start);
  }

  // Save original string representation once
  state.current_keyword->original_value_strings.push_back(repeat_text);

  DEBUG_OUT("[DEBUG] Repeat value: " << count << " x '" << value_str << "'\n");

  Value val = parse_text_to_value(value_str);
  for (int i = 0; i < count; ++i) {
    state.current_keyword->param_values.push_back(val);
  }
}

// ============================================================================
// COMMON ACTIONS - VALUE PARSING
// ============================================================================

// Generic value_item action (for backward compatibility)
template <> struct action<value_item> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, ParserState &state) {
    if (state.in_param_value && state.current_keyword) {
      add_value_to_current_keyword(in.string(), state);
    }
  }
};

// Generic value_list action
template <> struct action<value_list> {
  template <typename ActionInput>
  static void apply([[maybe_unused]] const ActionInput &in,
                    ParserState &state) {
    // Mark end of value list
    state.in_param_value = false;
    if (state.current_keyword) {
      DEBUG_OUT("[DEBUG] Completed value list for keyword '"
                << state.current_keyword->name << "' with "
                << state.current_keyword->param_values.size() << " values\n");
    }
  }
};

// ============================================================================
// REPEAT VALUE ACTIONS
// ============================================================================

// Action for repeat_real_value (N*number)
template <> struct action<repeat_real_value> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, ParserState &state) {
    if (state.in_param_value && state.current_keyword) {
      add_repeat_value_to_current_keyword(in.string(), state);
    }
  }
};

// Action for repeat_string_value (N*'string')
template <> struct action<repeat_string_value> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, ParserState &state) {
    if (state.in_param_value && state.current_keyword) {
      add_repeat_value_to_current_keyword(in.string(), state);
    }
  }
};

// ============================================================================
// TYPE-SPECIFIC VALUE ACTIONS
// ============================================================================

// String value action
template <> struct action<string_value> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, ParserState &state) {
    if (state.in_param_value && state.current_keyword) {
      add_value_to_current_keyword(in.string(), state);
    }
  }
};

// Integer value action
template <> struct action<integer_value> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, ParserState &state) {
    if (state.in_param_value && state.current_keyword) {
      add_value_to_current_keyword(in.string(), state);
    }
  }
};

// Real value action
template <> struct action<real_value> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, ParserState &state) {
    if (state.in_param_value && state.current_keyword) {
      add_value_to_current_keyword(in.string(), state);
    }
  }
};

// String list action (marks end of list)
template <> struct action<string_list> {
  template <typename ActionInput>
  static void apply([[maybe_unused]] const ActionInput &in,
                    ParserState &state) {
    state.in_param_value = false;
    if (state.current_keyword) {
      DEBUG_OUT("[DEBUG] Completed string list for keyword '"
                << state.current_keyword->name << "' with "
                << state.current_keyword->param_values.size() << " values\n");
    }
  }
};

// Integer list action (marks end of list)
template <> struct action<integer_list> {
  template <typename ActionInput>
  static void apply([[maybe_unused]] const ActionInput &in,
                    ParserState &state) {
    state.in_param_value = false;
    if (state.current_keyword) {
      DEBUG_OUT("[DEBUG] Completed integer list for keyword '"
                << state.current_keyword->name << "' with "
                << state.current_keyword->param_values.size() << " values\n");
    }
  }
};

// Real list action (marks end of list)
template <> struct action<real_list> {
  template <typename ActionInput>
  static void apply([[maybe_unused]] const ActionInput &in,
                    ParserState &state) {
    state.in_param_value = false;
    if (state.current_keyword) {
      DEBUG_OUT("[DEBUG] Completed real list for keyword '"
                << state.current_keyword->name << "' with "
                << state.current_keyword->param_values.size() << " values\n");
    }
  }
};

// ============================================================================
// AMBIGUOUS KEYWORD VALUE ACTIONS
// ============================================================================

// Safe value item action (for optional params - only numbers and quoted
// strings)
template <> struct action<safe_value_item> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, ParserState &state) {
    if (state.in_param_value && state.current_keyword) {
      // Check if this is a repeat value
      std::string text = in.string();
      if (text.find('*') != std::string::npos && 
          text.find('*') > 0 &&  // Not at start
          std::isdigit(text[0])) {  // Starts with digit
        add_repeat_value_to_current_keyword(text, state);
      } else {
        add_value_to_current_keyword(text, state);
      }
    }
  }
};

// Safe value list action (marks end of list)
template <> struct action<safe_value_list> {
  template <typename ActionInput>
  static void apply([[maybe_unused]] const ActionInput &in,
                    ParserState &state) {
    state.in_param_value = false;
    if (state.current_keyword) {
      DEBUG_OUT("[DEBUG] Completed safe value list for keyword '"
                << state.current_keyword->name << "' with "
                << state.current_keyword->param_values.size() << " values\n");
    }
  }
};

} // namespace actions
} // namespace dakota

#endif // DAKOTA_ACTIONS_COMMON_HPP
