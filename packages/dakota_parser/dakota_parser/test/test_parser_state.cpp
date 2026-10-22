// test_parser_state.cpp
// C++ unit tests for parser state across documents read in one process.
//
// Covers:
//   Suite 1 - ParserState (a block that fails to parse partway does not
//                          affect the next document)

#include <gtest/gtest.h>
#include <dakota_input_reader.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace {

const std::string k_study = R"(
environment
  tabular_data
method
  sampling
    samples = 10
    seed = 7
    model_pointer = 'M'
model
  id_model = 'M'
  single
variables
  uniform_uncertain = 2
    lower_bounds = 0.0 0.0
    upper_bounds = 1.0 1.0
    descriptors 'x1' 'x2'
interface
  analysis_drivers = 'text_book'
    direct
responses
  response_functions = 1
  no_gradients
  no_hessians
)";

// k_study with `after` followed by `added`: a keyword k_study lacks, then a
// line the block grammar rejects partway (not at the end of the block).
std::string broken_study(const std::string& after, const std::string& added) {
    std::string input = k_study;
    const auto pos = input.find(after);
    EXPECT_NE(pos, std::string::npos) << "'" << after << "' not in k_study";
    if (pos == std::string::npos) return input;
    input.insert(pos + after.size(), added);
    return input;
}

json read_to_json(const std::string& input, bool& ok) {
    json output;
    std::vector<std::string> errors;
    ok = dakota::read_freeform_input_string_to_json(input, output, errors);
    return output;
}

} // namespace

// ============================================================================
// Suite 1: ParserState
// ============================================================================

// 1.1  For each block: after a document whose block fails to parse partway,
//      the next document gives the same result as when read first.
TEST(ParserState, FailedBlockDoesNotLeakIntoNextDocument)
{
    struct Case { const char* block; const char* after; const char* added; };
    const std::vector<Case> cases = {
        {"environment", "  tabular_data\n",
         "  output_precision = 7\n  tabular_data_file = = 'x'\n"},
        {"method", "    seed = 7\n", "    fixed_seed\n    samples = '\n"},
        {"model", "  single\n", "  variables_pointer = 'V'\n  responses_pointer = '\n"},
        {"variables", "    descriptors 'x1' 'x2'\n",
         "  continuous_design = 1\n    descriptors = '\n"},
        {"interface", "    direct\n", "  asynchronous\n  analysis_drivers = '\n"},
        {"responses", "  no_hessians\n", "  descriptors = 'r1'\n  response_functions = '\n"},
    };

    bool ok = false;
    const json fresh = read_to_json(k_study, ok);
    ASSERT_TRUE(ok);

    for (const auto& c : cases) {
        read_to_json(broken_study(c.after, c.added), ok);
        EXPECT_FALSE(ok) << c.block << ": the broken document should not parse";

        const json again = read_to_json(k_study, ok);
        EXPECT_TRUE(ok) << c.block;
        EXPECT_EQ(again, fresh) << c.block;
    }
}
