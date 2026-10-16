// test_json_to_ast.cpp
// C++ unit tests for JSON input through the parser AST (json_to_ast).
//
// Covers:
//   Suite 1 - JsonToAstRoundTrip  (json_to_ast inverts ast_to_json; JSON input
//                                  gives the same result as freeform input)
//   Suite 2 - JsonToAstShape      (malformed JSON rejected; null is absent)

#include <gtest/gtest.h>
#include <ast_to_json.hpp>
#include <dakota_input_reader.hpp>
#include <dakota_parser.hpp>
#include <json_to_ast.hpp>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace {

const char* const k_study = R"(
environment
  tabular_data
method
  sampling
    samples = 10
    seed = 7
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

// The JSON that the freeform reader produces before default expansion.
// Blocks with no keywords are restored as {} (ast_to_json drops them).
bool unexpanded_json(dakota::Document doc, json& j0) {
    std::vector<std::string> errors;
    if (!dakota::validate_document(doc, errors) ||
        !dakota::analyze_semantics(doc, errors)) {
        return false;
    }
    j0 = dakota::ast_to_json(doc);
    for (const auto& block : doc.blocks) {
        if (j0.contains(block.name)) continue;
        if (dakota::allows_multiple_instances(block.name)) {
            j0[block.name] = json::array({json::object()});
        } else {
            j0[block.name] = json::object();
        }
    }
    return true;
}

json study_json() {
    dakota::Document doc;
    json j0;
    EXPECT_TRUE(dakota::parse_dakota_string(k_study, "<study>", doc));
    EXPECT_TRUE(unexpanded_json(doc, j0));
    return j0;
}

bool any_error_contains(const std::vector<std::string>& errors,
                        const std::string& text) {
    return std::any_of(errors.begin(), errors.end(), [&](const std::string& e) {
        return e.find(text) != std::string::npos;
    });
}

} // anonymous namespace

// ============================================================================
// Suite 1: JsonToAstRoundTrip
// ============================================================================

// 1.1  For every Dakota regression input that the freeform reader accepts:
//      ast_to_json(json_to_ast(J0)) == J0, and JSON input J0 gives the same
//      validated JSON as the freeform input.  Reads every input, so it is
//      the slowest test here (about 20 s in a Debug build).
TEST(JsonToAstRoundTrip, RegressionInputs)
{
    const std::filesystem::path dir = DAKOTA_TEST_INPUT_DIR;
    if (!std::filesystem::is_directory(dir)) {
        GTEST_SKIP() << "No Dakota test inputs at " << dir;
    }
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".in") files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());

    int checked = 0;
    for (const auto& file : files) {
        json freeform;
        std::vector<std::string> ff_errors;
        if (!dakota::read_freeform_input_file_to_json(file.string(), freeform, ff_errors)) {
            continue;
        }
        dakota::Document doc;
        json j0;
        ASSERT_TRUE(dakota::parse_dakota_file(file.string(), doc));
        ASSERT_TRUE(unexpanded_json(doc, j0)) << file;

        dakota::Document converted;
        std::vector<std::string> errors;
        ASSERT_TRUE(dakota::json_to_ast(j0, converted, errors))
            << file << ": " << (errors.empty() ? "" : errors[0]);
        json round_trip = dakota::ast_to_json(converted);
        for (auto& [name, block] : j0.items()) {
            if (!round_trip.contains(name)) round_trip[name] = block;  // restored empty block
        }
        EXPECT_EQ(round_trip, j0) << file;

        json output;
        EXPECT_TRUE(dakota::validate_json_input_to_json(j0, output, errors))
            << file << ": " << (errors.empty() ? "" : errors[0]);
        EXPECT_EQ(output, freeform) << file;
        ++checked;
    }
    EXPECT_GT(checked, 100) << "Too few inputs checked in " << dir;
}

// ============================================================================
// Suite 2: JsonToAstShape
// ============================================================================

// 2.1  A top-level key that is not a block is rejected, even when empty.
TEST(JsonToAstShape, UnknownBlockRejected)
{
    auto doc = study_json();
    doc["bogus_block"] = json::object();

    json output;
    std::vector<std::string> errors;
    EXPECT_FALSE(dakota::validate_json_input_to_json(doc, output, errors));
    EXPECT_TRUE(any_error_contains(errors, "Unexpected top-level block: bogus_block"));
}

// 2.2  Blocks that allow several instances must be arrays of objects.
TEST(JsonToAstShape, BlockArrayShapeChecked)
{
    auto doc = study_json();
    doc["method"] = doc["method"][0];

    dakota::Document ast;
    std::vector<std::string> errors;
    EXPECT_FALSE(dakota::json_to_ast(doc, ast, errors));
    EXPECT_TRUE(any_error_contains(errors, "method: expected an array of blocks"));

    json output;
    errors.clear();
    EXPECT_FALSE(dakota::validate_json_input_to_json(doc, output, errors));
    EXPECT_TRUE(any_error_contains(errors, "method: expected an array of blocks"));
}

// 2.3  A scalar where the schema wants an array is rejected before the
//      conversion, which could not tell the two apart.
TEST(JsonToAstShape, ScalarForArrayRejected)
{
    auto doc = study_json();
    doc["variables"][0]["uniform_uncertain"]["descriptors"] = "x1";

    json output;
    std::vector<std::string> errors;
    EXPECT_FALSE(dakota::validate_json_input_to_json(doc, output, errors));
    EXPECT_TRUE(any_error_contains(errors, "variables[0].uniform_uncertain"))
        << "Errors: " << (errors.empty() ? "(none)" : errors[0]);
}

// 2.4  Nested arrays are not valid parameter values.
TEST(JsonToAstShape, NestedArrayValueRejected)
{
    json doc = {{"method", json::array({{{"sampling", {{"seed", json::array({json::array({1})})}}}}})}};

    dakota::Document ast;
    std::vector<std::string> errors;
    EXPECT_FALSE(dakota::json_to_ast(doc, ast, errors));
    EXPECT_TRUE(any_error_contains(errors, "method[0].sampling.seed"))
        << "Errors: " << (errors.empty() ? "(none)" : errors[0]);
}

// 2.5  Integers above the int64 range become doubles, as in freeform input,
//      instead of wrapping around to negative values.
TEST(JsonToAstShape, LargeUnsignedIntegerKeptAsDouble)
{
    auto doc = study_json();
    doc["variables"][0]["uniform_uncertain"]["lower_bounds"] =
        json::array({10000000000000000000ULL, 0.0});
    doc["variables"][0]["uniform_uncertain"]["upper_bounds"] = json::array({2e19, 1.0});

    json output;
    std::vector<std::string> errors;
    ASSERT_TRUE(dakota::validate_json_input_to_json(doc, output, errors))
        << (errors.empty() ? "" : errors[0]);
    EXPECT_EQ(output["variables"][0]["uniform_uncertain"]["lower_bounds"][0].get<double>(),
              1e19);
}

// 2.6  A null value is the same as an absent key.
TEST(JsonToAstShape, NullIsAbsent)
{
    auto doc = study_json();
    json without_seed = doc;
    without_seed["method"][0]["sampling"].erase("seed");
    doc["method"][0]["sampling"]["seed"] = nullptr;

    json with_null, absent;
    std::vector<std::string> errors;
    ASSERT_TRUE(dakota::validate_json_input_to_json(doc, with_null, errors))
        << (errors.empty() ? "" : errors[0]);
    ASSERT_TRUE(dakota::validate_json_input_to_json(without_seed, absent, errors))
        << (errors.empty() ? "" : errors[0]);
    EXPECT_EQ(with_null, absent);
}
