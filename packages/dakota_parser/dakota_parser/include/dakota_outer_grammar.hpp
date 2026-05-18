#ifndef DAKOTA_OUTER_GRAMMAR_HPP
#define DAKOTA_OUTER_GRAMMAR_HPP

#include <tao/pegtl.hpp>

namespace dakota {
namespace outer {

using namespace tao::pegtl;

// ============================================================================
// Block Keywords
// ============================================================================

struct environment_kw : seq<
    TAO_PEGTL_STRING("environment"),
    not_at<sor<alnum, one<'_'>>>
> {};
struct interface_kw : seq<
    TAO_PEGTL_STRING("interface"),
    not_at<sor<alnum, one<'_'>>>
> {};
struct method_kw : seq<
    TAO_PEGTL_STRING("method"),
    not_at<sor<alnum, one<'_'>>>
> {};
struct model_kw : seq<
    TAO_PEGTL_STRING("model"),
    not_at<sor<alnum, one<'_'>>>
> {};
struct responses_kw : seq<
    TAO_PEGTL_STRING("responses"),
    not_at<sor<alnum, one<'_'>>>
> {};
struct variables_kw : seq<
    TAO_PEGTL_STRING("variables"),
    not_at<sor<alnum, one<'_'>>>
> {};

// Any block keyword
struct block_name : sor<
    environment_kw,
    interface_kw,
    method_kw,
    model_kw,
    responses_kw,
    variables_kw
> {};

// ============================================================================
// Whitespace and Comments
// ============================================================================

struct ws : sor<space, one<'\t'>> {};
struct opt_ws : star<ws> {};
struct comment : seq<one<'#'>, until<eol>> {};
struct opt_separator : star<sor<ws, eol, comment>> {};

// ============================================================================
// Block Content Extraction
// ============================================================================

// Complete identifier (for tokenized consumption)
struct identifier_token : seq<
    sor<alpha, one<'_'>>,
    star<sor<alnum, one<'_'>>>
> {};

// Content token: consume complete identifiers (unless they're block keywords) or other characters
struct content_token : sor<
    comment,                                     // Consume full comments (might contain block keyword text)
    seq<
        not_at<block_name>,                      // Not starting a block keyword
        identifier_token                         // Consume complete identifier (alpha/_ start)
    >,
    any                                          // Consume any other character (digits, =, etc.)
> {};

// Separator without comments (for termination checks - don't look inside comments)
struct opt_ws_eol : star<sor<ws, eol>> {};

// Block content: everything until next block or EOF
// Use opt_ws_eol (not opt_separator) to avoid looking inside comments for block keywords
struct block_content : until<
    sor<
        at<seq<opt_ws_eol, block_name>>,        // Stop at next block keyword (not in comments)
        eof                                      // Or end of file
    >,
    content_token                                // Consume in smart tokens (never splits identifiers)
> {};

// ============================================================================
// Block Structure
// ============================================================================

// Single block: optional separator + name + optional comma + content
struct block : seq<
    opt_separator,  // Allow whitespace/newlines before block name
    block_name,
    opt_separator,
    opt<one<','>>,
    block_content
> {};

// ============================================================================
// Top-Level Grammar
// ============================================================================

struct grammar : must<
    opt_separator,
    star<block>,
    opt_separator,
    eof
> {};

} // namespace outer
} // namespace dakota

#endif // DAKOTA_OUTER_GRAMMAR_HPP
