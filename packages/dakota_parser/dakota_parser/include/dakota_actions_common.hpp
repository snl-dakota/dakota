#ifndef DAKOTA_ACTIONS_COMMON_HPP
#define DAKOTA_ACTIONS_COMMON_HPP

#include "dakota_ast.hpp"
#include "dakota_grammar_common.hpp"
#include <algorithm>
#include <cerrno>
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
  // Try integer (no decimal, no exponent). Store as int64_t — the full
  // range of strtoll.  The old code cast to int and used int-range bounds,
  // which (a) used the wrong type (Value holds int64_t, not int) and
  // (b) silently fell through to double for large integers like 360000000000.
  if (text.find('.') == std::string::npos &&
      text.find('e') == std::string::npos &&
      text.find('E') == std::string::npos) {
    char *end = nullptr;
    errno = 0;
    long long iv = std::strtoll(text.c_str(), &end, 10);
    if (end == text.c_str() + text.size() && errno == 0) {
      return static_cast<int64_t>(iv);
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

// Parse a text token as a real (double).  Used for REAL/REALLIST contexts
// where integer-looking values like "1" or "-2" must be stored as double so
// that the DSL printer produces consistent output regardless of notation.
inline Value parse_real_to_value(const std::string& text) {
  auto quoted = unquote_and_trim(text);
  if (quoted) return *quoted;  // Shouldn't happen in a real context, but safe

  // Non-finite literals
  std::string lower = text;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  if (lower == "inf" || lower == "infinity")   return std::numeric_limits<double>::infinity();
  if (lower == "-inf" || lower == "-infinity") return -std::numeric_limits<double>::infinity();
  if (lower == "nan" || lower == "+nan" || lower == "-nan")
    return std::numeric_limits<double>::quiet_NaN();

  // Always parse as double (even for bare integers like "1" or "-2")
  char *end = nullptr;
  double dv = std::strtod(text.c_str(), &end);
  if (end == text.c_str() + text.size()) return dv;

  return text;  // Fallback: bare identifier
}

// Helper to add a real value to current keyword (forces double storage)
inline void add_real_to_current_keyword(const std::string& text, ParserState& state) {
  if (!state.current_keyword) return;
  state.current_keyword->original_value_strings.push_back(text);
  Value val = parse_real_to_value(text);
  DEBUG_OUT("[DEBUG] Real value: " << text << "\n");
  state.current_keyword->param_values.push_back(val);
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

// Expand a range literal "start:end" into a sequence of double values.
// Dakota range syntax: 5:50 → 5.0 6.0 7.0 ... 50.0 (step 1, always doubles).
inline void add_range_value_to_current_keyword(const std::string &range_text,
                                               ParserState &state) {
  if (!state.current_keyword) return;

  size_t colon = range_text.find(':');
  if (colon == std::string::npos) {
    add_value_to_current_keyword(range_text, state);
    return;
  }

  int64_t start_val = std::strtoll(range_text.c_str(), nullptr, 10);
  int64_t end_val   = std::strtoll(range_text.c_str() + colon + 1, nullptr, 10);

  // Store the original range text once for round-trip purposes
  state.current_keyword->original_value_strings.push_back(range_text);

  DEBUG_OUT("[DEBUG] Range value: " << start_val << ":" << end_val << "\n");

  // Expand: always produces doubles to match Dakota's ordered-output format
  int64_t step = (end_val >= start_val) ? 1 : -1;
  for (int64_t v = start_val; step > 0 ? v <= end_val : v >= end_val; v += step) {
    state.current_keyword->param_values.push_back(static_cast<double>(v));
  }
}

inline void add_range_step_value_to_current_keyword(const std::string &range_text,
                                                    ParserState &state) {
  if (!state.current_keyword) return;

  // Parse "start:step:end" — all components are real numbers.
  size_t c1 = range_text.find(':');
  size_t c2 = range_text.find(':', c1 + 1);
  if (c1 == std::string::npos || c2 == std::string::npos) {
    add_value_to_current_keyword(range_text, state);
    return;
  }

  double start_val = std::strtod(range_text.c_str(), nullptr);
  double step_val  = std::strtod(range_text.c_str() + c1 + 1, nullptr);
  double end_val   = std::strtod(range_text.c_str() + c2 + 1, nullptr);

  state.current_keyword->original_value_strings.push_back(range_text);
  DEBUG_OUT("[DEBUG] Range-step value: " << start_val << ":" << step_val << ":" << end_val << "\n");

  if (step_val == 0.0) {
    state.current_keyword->param_values.push_back(start_val);
    return;
  }
  // Expand with a small epsilon guard to handle floating-point edge cases.
  double eps = std::abs(step_val) * 1e-9;
  if (step_val > 0) {
    for (double v = start_val; v <= end_val + eps; v += step_val)
      state.current_keyword->param_values.push_back(v);
  } else {
    for (double v = start_val; v >= end_val - eps; v += step_val)
      state.current_keyword->param_values.push_back(v);
  }
}

// ============================================================================
// COMMON ACTIONS - VALUE PARSING
// ============================================================================

// Generic value_item action (for backward compatibility)
// Helper: does the string look like an integer:integer range (e.g. "5:50", "-3:3")?
// Does the string look like a repeat literal (e.g. "6*2.0", "3*'hello'")?
// Used to suppress double-processing in action<value_item>.
static inline bool is_repeat_literal(const std::string& s) {
  size_t star = s.find('*');
  if (star == std::string::npos || star == 0) return false;
  // Left side: optional blanks, then digits, then optional blanks before *
  // Handles both "9*2.0" and "9 * 2.0" forms
  size_t i = 0;
  while (i < star && (s[i] == ' ' || s[i] == '\t')) ++i;
  if (i == star) return false;
  while (i < star && std::isdigit((unsigned char)s[i])) ++i;
  while (i < star && (s[i] == ' ' || s[i] == '\t')) ++i;
  return i == star;
}

static inline bool is_range_literal(const std::string& s) {
  size_t colon = s.find(':');
  if (colon == std::string::npos || colon == 0 || colon + 1 == s.size()) return false;
  // Check left side: optional sign then digits only
  size_t i = 0;
  if (s[i] == '-' || s[i] == '+') ++i;
  if (i == colon) return false;  // no digits on left
  while (i < colon) { if (!std::isdigit((unsigned char)s[i])) return false; ++i; }
  // Check right side: optional sign then digits only
  size_t j = colon + 1;
  if (j < s.size() && (s[j] == '-' || s[j] == '+')) ++j;
  if (j == s.size()) return false;  // no digits on right
  while (j < s.size()) { if (!std::isdigit((unsigned char)s[j])) return false; ++j; }
  return true;
}

template <> struct action<value_item> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, ParserState &state) {
    if (state.in_param_value && state.current_keyword) {
      // Skip: range_value (e.g. "5:50") is handled by action<range_value>
      // which already expanded the range into individual param_values.
      // Without this guard, action<value_item> fires AFTER action<range_value>
      // and appends the raw "5:50" text as a spurious extra value.
      // We check the full integer:integer pattern to avoid skipping quoted
      // strings that happen to contain ':' (e.g. file paths in identifiers).
      if (is_range_literal(in.string()) || is_repeat_literal(in.string())) return;
      // Also skip start:step:end range — handled by action<range_value_step>
      {
        const std::string& _s = in.string();
        size_t _c1 = _s.find(':'), _c2 = _c1 != std::string::npos ? _s.find(':', _c1+1) : std::string::npos;
        if (_c1 != std::string::npos && _c2 != std::string::npos) return;
      }
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

// Range value action (start:end → sequence of doubles)
template <> struct action<range_value> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, ParserState &state) {
    if (state.in_param_value && state.current_keyword) {
      add_range_value_to_current_keyword(in.string(), state);
    }
  }
};

template <> struct action<range_value_step> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, ParserState &state) {
    if (state.in_param_value && state.current_keyword) {
      add_range_step_value_to_current_keyword(in.string(), state);
    }
  }
};

// Real value action — forces double storage so "1" and "1.0" are identical
template <> struct action<real_value> {
  template <typename ActionInput>
  static void apply(const ActionInput &in, ParserState &state) {
    if (state.in_param_value && state.current_keyword) {
      add_real_to_current_keyword(in.string(), state);
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
      const std::string& text = in.string();
      // Repeat literals are handled by action<repeat_real_value> /
      // action<repeat_string_value> — skip to avoid double-add.
      if (is_repeat_literal(text)) return;
      // Numeric values and infinity literals are handled by action<real_value>
      // (which fires for the real_value sub-rule of safe_value_item) — skip.
      // Quoted strings are NOT handled by any sub-rule action, so pass them through.
      if (text.empty()) return;
      char first = text[0];
      bool is_numeric_or_inf = (first == '-' || first == '+' ||
                                 std::isdigit((unsigned char)first) ||
                                 first == '.' ||
                                 first == 'i' || first == 'I' ||  // inf/Inf
                                 first == 'n' || first == 'N');   // nan/NaN
      if (is_numeric_or_inf) return;  // action<real_value> already stored it
      // Only quoted strings reach here
      add_value_to_current_keyword(text, state);
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
