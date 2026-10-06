// test_value_expansion.cpp
// C++ unit tests for expansion of repeat (N*value) and range (start:end,
// start:step:end) value syntax in freeform Dakota input.
//
// Covers:
//   Suite 1 - RepeatRange      (N*range: range expanded, then repeated N times)
//   Suite 2 - ExistingForms    (repeat and range forms on their own are unchanged)
//   Suite 3 - RepeatRangeJson  (whole-file read through validation to JSON)

#include <gtest/gtest.h>
#include <dakota_parser.hpp>
#include <dakota_input_reader.hpp>
#include <nlohmann/json.hpp>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

using json = nlohmann::json;

// ============================================================================
// Helpers
// ============================================================================

namespace {

// Depth-first search for the first keyword named `name` in `keywords`.
std::shared_ptr<dakota::KeywordNode> find_keyword(
    const std::map<std::string, std::vector<std::shared_ptr<dakota::KeywordNode>>>& keywords,
    const std::string& name)
{
    for (const auto& [kw_name, nodes] : keywords) {
        for (const auto& node : nodes) {
            if (kw_name == name) return node;
            if (auto child = find_keyword(node->children, name)) return child;
        }
    }
    return nullptr;
}

// Parse `input` (grammar only, no validation) and return the numeric values
// of keyword `name` in block `block`.  Fails the test if anything is missing.
std::vector<double> parsed_values(const std::string& input,
                                  const std::string& block,
                                  const std::string& name)
{
    dakota::Document doc;
    EXPECT_TRUE(dakota::parse_dakota_string(input, "test_input", doc));
    for (const auto& b : doc.blocks) {
        if (b.name != block) continue;
        auto kw = find_keyword(b.keywords, name);
        EXPECT_NE(kw, nullptr) << "keyword '" << name << "' not found";
        if (!kw) return {};
        std::vector<double> values;
        for (const auto& v : kw->param_values) {
            if (std::holds_alternative<double>(v))
                values.push_back(std::get<double>(v));
            else if (std::holds_alternative<int64_t>(v))
                values.push_back(static_cast<double>(std::get<int64_t>(v)));
            else
                ADD_FAILURE() << "non-numeric value in '" << name << "'";
        }
        return values;
    }
    ADD_FAILURE() << "block '" << block << "' not found";
    return {};
}

std::string method_with_points(const std::string& points)
{
    return "method\n  list_parameter_study\n    list_of_points " + points + "\n";
}

std::vector<double> points(const std::string& text)
{
    return parsed_values(method_with_points(text), "method", "list_of_points");
}

std::vector<double> repeated(const std::vector<double>& seq, int count)
{
    std::vector<double> out;
    for (int i = 0; i < count; ++i) out.insert(out.end(), seq.begin(), seq.end());
    return out;
}

// Input file with the combined syntax; `points` replaces the list_of_points
// values.  Three variables, so the point count must be a multiple of 3.
std::string combined_input(const std::string& points)
{
    return "environment\n"
           "  tabular_data\n"
           "method\n"
           "  list_parameter_study\n"
           "    list_of_points " + points + "    # trailing comment\n"
           "variables\n"
           "  uniform_uncertain 3\n"
           "  lower_bounds 3*-1\n"
           "  upper_bounds 3*1\n"
           "responses\n"
           "  response_functions 1\n"
           "  no_gradients\n"
           "  no_hessians\n"
           "interface\n"
           "  direct\n"
           "    analysis_drivers 'text_book'\n";
}

} // namespace

// ============================================================================
// Suite 1 - RepeatRange
// ============================================================================

TEST(RepeatRange, StepRangeRepeatedAsSequence)
{
    const std::vector<double> range = points("-1:0.2:1");
    ASSERT_EQ(range.size(), 11u);
    EXPECT_EQ(points("3 * -1:0.2:1"), repeated(range, 3));
}

TEST(RepeatRange, FollowedByPlainValues)
{
    std::vector<double> expected = repeated(points("-1:0.2:1"), 3);
    expected.insert(expected.end(), {-1.0, 0.0, 1.0});
    EXPECT_EQ(points("3 * -1:0.2:1 -1 0 1"), expected);
}

TEST(RepeatRange, BlanksAroundStarOptional)
{
    const std::vector<double> expected = repeated(points("-1:0.2:1"), 3);
    EXPECT_EQ(points("3*-1:0.2:1"), expected);
    EXPECT_EQ(points("3 *-1:0.2:1"), expected);
    EXPECT_EQ(points("3* -1:0.2:1"), expected);
}

TEST(RepeatRange, IntegerRangeInRealList)
{
    EXPECT_EQ(points("2*1:3"), (std::vector<double>{1, 2, 3, 1, 2, 3}));
    EXPECT_EQ(points("2*3:1"), (std::vector<double>{3, 2, 1, 3, 2, 1}));
}

TEST(RepeatRange, IntegerRangeInIntegerList)
{
    const std::string input =
        "responses\n"
        "  response_functions 6\n"
        "  mixed_gradients\n"
        "    id_numerical_gradients 2*1:3\n"
        "    method_source dakota\n"
        "  no_hessians\n";
    EXPECT_EQ(parsed_values(input, "responses", "id_numerical_gradients"),
              (std::vector<double>{1, 2, 3, 1, 2, 3}));
}

TEST(RepeatRange, GenericValueList)
{
    // initial_point uses the generic value list, not a typed real list.
    const std::string input =
        "variables\n"
        "  continuous_design 6\n"
        "    initial_point 2*1:2 1:2\n";
    EXPECT_EQ(parsed_values(input, "variables", "initial_point"),
              (std::vector<double>{1, 2, 1, 2, 1, 2}));
}

TEST(RepeatRange, MixedWithOtherForms)
{
    std::vector<double> expected = {0.5, 0.5};
    const std::vector<double> rr = repeated(std::vector<double>{1, 2}, 2);
    expected.insert(expected.end(), rr.begin(), rr.end());
    expected.insert(expected.end(), {4, 5});
    EXPECT_EQ(points("2*0.5 2*1:2 4:5"), expected);
}

// ============================================================================
// Suite 2 - ExistingForms
// ============================================================================

TEST(ExistingForms, RepeatValue)
{
    EXPECT_EQ(points("3*1.5"), (std::vector<double>{1.5, 1.5, 1.5}));
    EXPECT_EQ(points("3 * -1"), (std::vector<double>{-1, -1, -1}));
}

TEST(ExistingForms, IntegerRange)
{
    EXPECT_EQ(points("1:3"), (std::vector<double>{1, 2, 3}));
    EXPECT_EQ(points("-1:1"), (std::vector<double>{-1, 0, 1}));
}

TEST(ExistingForms, StepRange)
{
    const std::vector<double> v = points("1000.:1000.:3000.");
    EXPECT_EQ(v, (std::vector<double>{1000., 2000., 3000.}));
    EXPECT_EQ(points("-1:0.2:1").size(), 11u);
}

TEST(ExistingForms, StepRangeNoAccumulatedDrift)
{
    // Values are computed from the index, not a running sum, so the value
    // that lands on zero is exactly zero.
    const std::vector<double> v = points("-1:0.2:1");
    ASSERT_EQ(v.size(), 11u);
    for (size_t i = 1; i < v.size(); ++i) {
        const double steps = static_cast<double>(i) - 5.0;
        EXPECT_EQ(v[i], steps * 0.2) << "i = " << i;
    }
    EXPECT_EQ(v[5], 0.0);
    EXPECT_EQ(v[10], 1.0);
    EXPECT_EQ(points("2*-1:0.2:1"), repeated(v, 2));
}

TEST(ExistingForms, StepRangeEndpointWithinRounding)
{
    // 3*0.1 is 0.30000000000000004; the endpoint is kept and set to end.
    EXPECT_EQ(points("0:0.1:0.3"), (std::vector<double>{0.0, 0.1, 0.2, 0.3}));
    const std::vector<double> down = points("1:-0.1:0");
    ASSERT_EQ(down.size(), 11u);
    EXPECT_EQ(down.back(), 0.0);
}

TEST(ExistingForms, StepRangeStartAsWritten)
{
    // (0.7/0.3)*0.3 is 0.70000000000000007; the first value is start itself.
    const std::vector<double> v = points("0.7:0.3:2");
    ASSERT_EQ(v.size(), 5u);
    EXPECT_EQ(v.front(), 0.7);
}

TEST(ExistingForms, StepRangeNegativeStepCrossesPositiveZero)
{
    const std::vector<double> v = points("1:-0.5:-1");
    ASSERT_EQ(v.size(), 5u);
    EXPECT_EQ(v, (std::vector<double>{1.0, 0.5, 0.0, -0.5, -1.0}));
    EXPECT_FALSE(std::signbit(v[2]));
}

TEST(ExistingForms, RepeatString)
{
    dakota::Document doc;
    ASSERT_TRUE(dakota::parse_dakota_string(
        "interface\n  direct\n    analysis_drivers 2*'text_book'\n",
        "test_input", doc));
    ASSERT_EQ(doc.blocks.size(), 1u);
    auto kw = find_keyword(doc.blocks[0].keywords, "analysis_drivers");
    ASSERT_NE(kw, nullptr);
    ASSERT_EQ(kw->param_values.size(), 2u);
    for (const auto& v : kw->param_values) {
        ASSERT_TRUE(std::holds_alternative<std::string>(v));
        EXPECT_EQ(std::get<std::string>(v), "text_book");
    }
}

// ============================================================================
// Suite 3 - RepeatRangeJson
// ============================================================================

TEST(RepeatRangeJson, WholeFileRepeatedRange)
{
    const json out = dakota::read_freeform_input_string_to_json(
        combined_input("3 * -1:0.2:1"));
    const auto& lop = out["method"][0]["list_parameter_study"]["source"]["list_of_points"];
    ASSERT_TRUE(lop.is_array()) << out["method"].dump();
    EXPECT_EQ(lop.size(), 33u);
    EXPECT_EQ(lop.get<std::vector<double>>(), repeated(points("-1:0.2:1"), 3));
}

TEST(RepeatRangeJson, WholeFileRepeatedRangeThenValues)
{
    const json out = dakota::read_freeform_input_string_to_json(
        combined_input("3 * -1:0.2:1 -1 0 1"));
    const auto& lop = out["method"][0]["list_parameter_study"]["source"]["list_of_points"];
    ASSERT_TRUE(lop.is_array()) << out["method"].dump();
    std::vector<double> expected = repeated(points("-1:0.2:1"), 3);
    expected.insert(expected.end(), {-1.0, 0.0, 1.0});
    EXPECT_EQ(lop.size(), 36u);
    EXPECT_EQ(lop.get<std::vector<double>>(), expected);
}
