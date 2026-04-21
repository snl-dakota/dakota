#ifndef DAKOTA_GRAMMAR_COMMON_HPP
#define DAKOTA_GRAMMAR_COMMON_HPP

#include <tao/pegtl.hpp>

namespace pegtl = tao::pegtl;

namespace dakota {

// ============================================================================
// BASIC ELEMENTS
// ============================================================================

// Comments: '#' to end of line
struct comment : pegtl::seq<pegtl::one<'#'>, pegtl::until<pegtl::eol>> {};

// Whitespace and separators
struct ws : pegtl::plus<pegtl::space> {};
struct opt_ws : pegtl::star<pegtl::space> {};
struct separator : pegtl::sor<ws, comment> {};
struct opt_separator : pegtl::star<pegtl::sor<pegtl::space, comment>> {};

// Identifiers: start with alpha, followed by alnum or underscore
struct identifier : pegtl::seq<
    pegtl::alpha,
    pegtl::star<pegtl::sor<pegtl::alnum, pegtl::one<'_'>>>
> {};

// Numbers: support integers, floats, scientific notation
struct number : pegtl::seq<
    pegtl::opt<pegtl::one<'-', '+'>>,
    pegtl::sor<
        pegtl::seq<pegtl::plus<pegtl::digit>, pegtl::opt<pegtl::one<'.'>, pegtl::star<pegtl::digit>>>,
        pegtl::seq<pegtl::one<'.'>, pegtl::plus<pegtl::digit>>
    >,
    pegtl::opt<pegtl::one<'e', 'E'>, pegtl::opt<pegtl::one<'-', '+'>>, pegtl::plus<pegtl::digit>>
> {};

// Integer only (no decimal point)
struct integer : pegtl::seq<
    pegtl::opt<pegtl::one<'-', '+'>>,
    pegtl::plus<pegtl::digit>
> {};

// Positive integer (for repeat counts - no sign allowed)
struct positive_integer : pegtl::plus<pegtl::digit> {};

// Infinity literals: inf, -inf, infinity, -infinity (case-insensitive)
struct infinity_literal : pegtl::seq<
    pegtl::opt<pegtl::one<'-', '+'>>,
    pegtl::sor<
        TAO_PEGTL_ISTRING("infinity"),
        TAO_PEGTL_ISTRING("inf")
    >,
    pegtl::not_at<pegtl::sor<pegtl::alnum, pegtl::one<'_'>>>
> {};

// Quoted strings: single or double quotes
struct single_quoted_string : pegtl::seq<
    pegtl::one<'\''>,
    pegtl::star<pegtl::not_one<'\''>>,
    pegtl::one<'\''>
> {};

struct double_quoted_string : pegtl::seq<
    pegtl::one<'"'>,
    pegtl::star<pegtl::not_one<'"'>>,
    pegtl::one<'"'>
> {};

struct quoted_string : pegtl::sor<single_quoted_string, double_quoted_string> {};

// ============================================================================
// REPEAT VALUE SUPPORT (N*value syntax)
// ============================================================================

// Repeat count - captures the N in N*value
struct repeat_count : positive_integer {};

// Repeat operator
struct repeat_operator : pegtl::one<'*'> {};

// Base values that can be repeated
struct repeat_base_number : pegtl::sor<infinity_literal, number> {};
struct repeat_base_string : quoted_string {};

// Repeat value patterns: N*value (optional space after *)
// Real repeat: 9*-1.0, 9* 1.0, 3*1.5e-3
struct repeat_real_value : pegtl::seq<
    repeat_count,
    repeat_operator,
    pegtl::star<pegtl::blank>,  // Allow optional spaces/tabs after *
    repeat_base_number
> {};

// String repeat: 3*'hello', 5* "world"
struct repeat_string_value : pegtl::seq<
    repeat_count,
    repeat_operator,
    pegtl::star<pegtl::blank>,  // Allow optional spaces/tabs after *
    repeat_base_string
> {};

// Combined repeat value (tries real first since it's more specific)
struct repeat_value : pegtl::sor<repeat_string_value, repeat_real_value> {};

// ============================================================================
// TYPE-SPECIFIC VALUE ITEMS
// ============================================================================

// String values MUST be quoted - prevents keyword/value ambiguity
struct string_value : quoted_string {};

// Integer values
struct integer_value : integer {};

// Real (float) values — includes infinity literals
struct real_value : pegtl::sor<infinity_literal, number> {};

// Filename values - must be quoted
struct filename_value : quoted_string {};

// ============================================================================
// TYPE-SPECIFIC VALUE LISTS (with repeat support)
// ============================================================================

struct keyword_start : pegtl::sor<
    pegtl::alpha,
    pegtl::one<'_'>
> {};

// Value separator: whitespace, commas, and/or comments between values
// This allows multi-line value lists with inline comments
struct value_separator : pegtl::plus<pegtl::sor<
    pegtl::space,
    pegtl::one<','>,
    comment
>> {};

// Optional value separator
struct opt_value_separator : pegtl::star<pegtl::sor<
    pegtl::space,
    pegtl::one<','>,
    comment
>> {};

// Value terminator: stops when we see a new keyword or block
// Uses opt_value_separator to skip comments before checking for keywords
struct value_terminator : pegtl::sor<
    pegtl::at<pegtl::seq<opt_value_separator, pegtl::sor<
        pegtl::seq<TAO_PEGTL_STRING("environment"), pegtl::not_at<pegtl::sor<pegtl::alnum, pegtl::one<'_'>>>>,
        pegtl::seq<TAO_PEGTL_STRING("method"), pegtl::not_at<pegtl::sor<pegtl::alnum, pegtl::one<'_'>>>>,
        pegtl::seq<TAO_PEGTL_STRING("model"), pegtl::not_at<pegtl::sor<pegtl::alnum, pegtl::one<'_'>>>>,
        pegtl::seq<TAO_PEGTL_STRING("variables"), pegtl::not_at<pegtl::sor<pegtl::alnum, pegtl::one<'_'>>>>,
        pegtl::seq<TAO_PEGTL_STRING("interface"), pegtl::not_at<pegtl::sor<pegtl::alnum, pegtl::one<'_'>>>>,
        pegtl::seq<TAO_PEGTL_STRING("responses"), pegtl::not_at<pegtl::sor<pegtl::alnum, pegtl::one<'_'>>>>
    >>>,
    pegtl::at<pegtl::seq<opt_value_separator, keyword_start>>
> {};

// String list: one or more quoted strings (with repeat support)
struct string_or_repeat_string : pegtl::sor<repeat_string_value, string_value> {};

struct string_list_items : pegtl::seq<
    string_or_repeat_string,
    pegtl::star<
        pegtl::seq<
            pegtl::not_at<value_terminator>,
            value_separator,
            pegtl::not_at<value_terminator>,
            opt_value_separator,
            pegtl::opt<string_or_repeat_string>
        >
    >,
    pegtl::star<pegtl::sor<pegtl::blank, pegtl::one<','>>>
> {};

// Wrapper for string list to provide action hook
struct string_list : string_list_items {};

// Integer list: one or more integers (with repeat support)
// Note: repeat_real_value handles integers too (9*0 parses as 9*0.0 which converts to int)
struct integer_or_repeat : pegtl::sor<repeat_real_value, integer_value> {};

struct integer_list_items : pegtl::seq<
    integer_or_repeat,
    pegtl::star<
        pegtl::seq<
            pegtl::not_at<value_terminator>,
            value_separator,
            pegtl::not_at<value_terminator>,
            opt_value_separator,
            pegtl::opt<integer_or_repeat>
        >
    >,
    pegtl::star<pegtl::sor<pegtl::blank, pegtl::one<','>>>
> {};

// Wrapper for integer list to provide action hook
struct integer_list : integer_list_items {};

// Real list: one or more real numbers (with repeat support)
struct real_or_repeat : pegtl::sor<repeat_real_value, real_value> {};

struct real_list_items : pegtl::seq<
    real_or_repeat,
    pegtl::star<
        pegtl::seq<
            pegtl::not_at<value_terminator>,
            value_separator,
            pegtl::not_at<value_terminator>,
            opt_value_separator,
            pegtl::opt<real_or_repeat>
        >
    >,
    pegtl::star<pegtl::sor<pegtl::blank, pegtl::one<','>>>
> {};

// Wrapper for real list to provide action hook
struct real_list : real_list_items {};

// Generic value item (for backwards compatibility or unknown types)
// NOTE: This includes identifier which can cause ambiguity - use type-specific rules when possible
struct value_item : pegtl::sor<
    repeat_value,
    quoted_string,
    number,
    identifier
> {};

// Generic value list (for backwards compatibility)
struct value_list : pegtl::seq<
    value_item,
    pegtl::star<
        pegtl::seq<
            pegtl::not_at<value_terminator>,
            value_separator,
            pegtl::not_at<value_terminator>,
            opt_value_separator,
            pegtl::opt<value_item>
        >
    >,
    pegtl::star<pegtl::sor<pegtl::blank, pegtl::one<','>>>
> {};

// Optional equals sign with surrounding whitespace
struct equals_with_ws : pegtl::seq<opt_ws, pegtl::one<'='>, opt_ws> {};

// ============================================================================
// KEYWORD TEMPLATES
// ============================================================================

// Note: Using must<> after keyword match ensures that the value MUST parse
// correctly. This prevents backtracking while still allowing actions to fire
// on both the keyword and value rules.

// Keyword with STRING parameter (must be quoted)
template<typename KeywordRule>
struct keyword_with_string : pegtl::seq<
    KeywordRule,
    pegtl::must<pegtl::sor<
        pegtl::seq<equals_with_ws, string_value>,
        pegtl::seq<pegtl::plus<pegtl::space>, string_value>
    >>
> {};

// Keyword with STRINGLIST parameter (must be quoted)
template<typename KeywordRule>
struct keyword_with_stringlist : pegtl::seq<
    KeywordRule,
    pegtl::must<pegtl::sor<
        pegtl::seq<equals_with_ws, string_list>,
        pegtl::seq<pegtl::plus<pegtl::space>, string_list>
    >>
> {};

// Keyword with INTEGER parameter
template<typename KeywordRule>
struct keyword_with_integer : pegtl::seq<
    KeywordRule,
    pegtl::must<pegtl::sor<
        pegtl::seq<equals_with_ws, integer_value>,
        pegtl::seq<pegtl::plus<pegtl::space>, integer_value>
    >>
> {};

// Keyword with INTEGERLIST parameter
template<typename KeywordRule>
struct keyword_with_integerlist : pegtl::seq<
    KeywordRule,
    pegtl::must<pegtl::sor<
        pegtl::seq<equals_with_ws, integer_list>,
        pegtl::seq<pegtl::plus<pegtl::space>, integer_list>
    >>
> {};

// Keyword with REAL parameter
template<typename KeywordRule>
struct keyword_with_real : pegtl::seq<
    KeywordRule,
    pegtl::must<pegtl::sor<
        pegtl::seq<equals_with_ws, real_value>,
        pegtl::seq<pegtl::plus<pegtl::space>, real_value>
    >>
> {};

// Keyword with REALLIST parameter
template<typename KeywordRule>
struct keyword_with_reallist : pegtl::seq<
    KeywordRule,
    pegtl::must<pegtl::sor<
        pegtl::seq<equals_with_ws, real_list>,
        pegtl::seq<pegtl::plus<pegtl::space>, real_list>
    >>
> {};

// Generic keyword with parameter (uses generic value_list - may cause ambiguity)
template<typename KeywordRule>
struct keyword_with_param : pegtl::seq<
    KeywordRule,
    pegtl::must<pegtl::sor<
        pegtl::seq<equals_with_ws, value_list>,
        pegtl::seq<pegtl::plus<pegtl::space>, value_list>
    >>
> {};

// Keyword that is just a flag (no parameter)
template<typename KeywordRule>
struct keyword_flag : KeywordRule {};

// ============================================================================
// AMBIGUOUS KEYWORD HANDLING
// ============================================================================

// Value item that doesn't include bare identifiers (prevents keyword/value ambiguity)
// Includes repeat support
struct safe_value_item : pegtl::sor<
    repeat_value,
    quoted_string,
    number
> {};

// Safe value list - only numbers and quoted strings, no bare identifiers
struct safe_value_list : pegtl::seq<
    safe_value_item,
    pegtl::star<
        pegtl::seq<
            pegtl::not_at<value_terminator>,
            value_separator,
            pegtl::not_at<value_terminator>,
            opt_value_separator,
            pegtl::opt<safe_value_item>
        >
    >,
    pegtl::star<pegtl::sor<pegtl::blank, pegtl::one<','>>>
> {};

// Keyword with OPTIONAL parameter - for ambiguous keywords that appear as both
// flag and param in different contexts. Uses lookahead to avoid consuming next keyword.
// Semantic analysis will validate whether value was required/allowed.
template<typename KeywordRule>
struct keyword_with_optional_param : pegtl::seq<
    KeywordRule,
    pegtl::opt<pegtl::sor<
        // With equals sign - definitely has a value
        pegtl::seq<equals_with_ws, safe_value_list>,
        // Without equals - only parse value if next thing looks like a value (number or quote)
        pegtl::seq<
            pegtl::plus<pegtl::space>,
            pegtl::at<pegtl::sor<
                pegtl::one<'"', '\''>,  // Quote for string
                pegtl::seq<pegtl::opt<pegtl::one<'-', '+'>>, pegtl::sor<pegtl::digit, pegtl::one<'.'>>>  // Number
            >>,
            safe_value_list
        >
    >>
> {};

} // namespace dakota

#endif // DAKOTA_GRAMMAR_COMMON_HPP
