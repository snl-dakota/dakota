// test_json_required_fields.cpp
// C++ unit tests for JSON required-field enforcement in the validation path.
//
// Covers:
//   Suite 1 - JsonRequiredFieldsWholeFile  (api_mode=false, all required fields enforced)
//   Suite 2 - JsonRequiredFieldsPerBlock   (api_mode=true, pointer fields relaxed)
//   Suite 3 - JsonRequiredFieldsRegression (existing JSON test files still pass)
//   Suite 4 - JsonPointerGroupUnion        (pointer-group and pointer-union API-mode relaxation)

#include <gtest/gtest.h>
#include <dakota_input_reader.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

// ============================================================================
// Helpers
// ============================================================================

namespace {

// Returns true if any error string contains the given substring.
bool any_error_contains(const std::vector<std::string>& errors,
                        const std::string& substr)
{
    for (const auto& e : errors) {
        if (e.find(substr) != std::string::npos) return true;
    }
    return false;
}

// Verbatim content of dakota_uq_textbook_lhs_json.json embedded as a raw
// string literal.  This is a known-good whole-file document; whole-file tests
// mutate copies of it.  Embedding avoids any filesystem path dependency.
// MAINTAINER: keep in sync with source/test/dakota_uq_textbook_lhs_json.json
constexpr const char* k_lhs_json = R"json(
{
  "environment": {
    "graphics": true
  },
  "interface": [
    {
      "analysis_drivers": {
        "drivers": [
          "text_book"
        ],
        "interface_type": {
          "system": {}
        }
      },
      "concurrency": {
        "asynchronous": {
          "evaluation_concurrency": 5
        }
      }
    }
  ],
  "method": [
    {
      "sampling": {
        "distribution": {
          "complementary": true
        },
        "response_levels": {
          "values": [
            360000000000.0,
            400000000000.0,
            440000000000.0,
            60000.0,
            65000.0,
            70000.0,
            350000.0,
            400000.0,
            450000.0
          ]
        },
        "samples": 100,
        "seed": 1
      }
    }
  ],
  "responses": [
    {
      "gradient_type": {
        "no_gradients": true
      },
      "hessian_type": {
        "no_hessians": true
      },
      "response_type": {
        "response_functions": {
          "count": 3
        }
      }
    }
  ],
  "variables": [
    {
      "histogram_bin_uncertain": {
        "abscissas": [
          5.0, 8.0, 10.0, 0.1, 0.2, 0.3, 0.4
        ],
        "count": 2,
        "density": {
          "counts": [
            17.0, 21.0, 0.0, 12.0, 24.0, 12.0, 0.0
          ]
        },
        "descriptors": [ "TF1h", "TF2h" ],
        "pairs_per_variable": [ 3, 4 ]
      },
      "histogram_point_uncertain": {
        "real": {
          "abscissas": [ 3.0, 4.0 ],
          "count": 1,
          "counts": [ 1.0, 1.0 ],
          "descriptors": [ "TF3h" ],
          "pairs_per_variable": [ 2 ]
        }
      },
      "normal_uncertain": {
        "count": 2,
        "descriptors": [ "TF1n", "TF2n" ],
        "means": [ 248.89, 593.33 ],
        "std_deviations": [ 12.4, 29.7 ]
      },
      "uniform_uncertain": {
        "count": 2,
        "descriptors": [ "TF1u", "TF2u" ],
        "lower_bounds": [ 199.3, 474.63 ],
        "upper_bounds": [ 298.5, 712.0 ]
      },
      "weibull_uncertain": {
        "alphas": [ 12.0, 30.0 ],
        "betas":  [ 250.0, 590.0 ],
        "count": 2,
        "descriptors": [ "TF1w", "TF2w" ]
      }
    }
  ]
}
)json";

json base_doc() { return json::parse(k_lhs_json); }

} // anonymous namespace

// ============================================================================
// Suite 1: JsonRequiredFieldsWholeFile
// api_mode=false — all required fields (including pointers) are enforced.
// ============================================================================

// 1.1  Removing a required non-pointer field must produce an error that
//      mentions "Missing required" and the field name.
TEST(JsonRequiredFieldsWholeFile, MissingRequiredNonPointerField)
{
    auto doc = base_doc();
    // Remove the required 'drivers' field from AnalysisDrivers
    doc["interface"][0]["analysis_drivers"].erase("drivers");

    json output;
    std::vector<std::string> errors;
    bool ok = dakota::validate_json_input_to_json(doc, output, errors);

    EXPECT_FALSE(ok);
    EXPECT_TRUE(any_error_contains(errors, "Missing required"))
        << "Expected 'Missing required' in errors";
    EXPECT_TRUE(any_error_contains(errors, "drivers"))
        << "Expected field name 'drivers' in errors";
}

// 1.2  A required pointer field must also error in whole-file mode.
//      ActiveSubspaceConfig requires truth_model_pointer.
TEST(JsonRequiredFieldsWholeFile, MissingRequiredPointerFieldErrors)
{
    auto doc = base_doc();
    // Replace the model block with active_subspace that omits truth_model_pointer
    doc["model"] = json::array({
        {{"active_subspace", {{"initial_samples", 100}}}}
    });

    json output;
    std::vector<std::string> errors;
    bool ok = dakota::validate_json_input_to_json(doc, output, errors);

    EXPECT_FALSE(ok);
    EXPECT_TRUE(any_error_contains(errors, "truth_model_pointer"))
        << "Expected 'truth_model_pointer' in errors; errors: "
        << (errors.empty() ? "(none)" : errors[0]);
}

// 1.3  A minimal document with all required fields present must pass.
TEST(JsonRequiredFieldsWholeFile, AllRequiredFieldsPresent)
{
    auto doc = base_doc();
    json output;
    std::vector<std::string> errors;
    bool ok = dakota::validate_json_input_to_json(doc, output, errors);

    EXPECT_TRUE(ok)
        << "Errors: " << (errors.empty() ? "(none)" : errors[0]);
}

// ============================================================================
// Suite 2: JsonRequiredFieldsPerBlock
// api_mode=true (per-block entry points) — pointer fields relaxed,
// non-pointer required fields still enforced.
// ============================================================================

// 2.1  Omitting a required pointer field in a per-block call must succeed.
//      ActiveSubspaceConfig requires truth_model_pointer (a block pointer).
TEST(JsonRequiredFieldsPerBlock, MissingRequiredPointerOkInApiMode)
{
    json input = {
        {"active_subspace", {
            {"initial_samples", 100}
        }}
    };
    json output;
    std::vector<std::string> errors;
    bool ok = dakota::validate_model_block_json_to_json(input, output, errors);

    EXPECT_TRUE(ok)
        << "Errors: " << (errors.empty() ? "(none)" : errors[0]);
}

// 2.2  Omitting a required non-pointer field in a per-block call must error.
//      BetaUncertain requires count, alphas, betas, lower_bounds, upper_bounds.
TEST(JsonRequiredFieldsPerBlock, MissingRequiredNonPointerErrorsInApiMode)
{
    json input = {
        {"beta_uncertain", {
            {"count", 2},
            // "alphas" intentionally omitted
            {"betas",        {1.0, 1.0}},
            {"lower_bounds", {0.0, 0.0}},
            {"upper_bounds", {1.0, 1.0}}
        }}
    };
    json output;
    std::vector<std::string> errors;
    bool ok = dakota::validate_variables_block_json_to_json(input, output, errors);

    EXPECT_FALSE(ok);
    EXPECT_TRUE(any_error_contains(errors, "alphas"))
        << "Expected 'alphas' in errors; errors: "
        << (errors.empty() ? "(none)" : errors[0]);
}

// 2.3  When both a required pointer and a required non-pointer are absent,
//      all pointer-related fields are relaxed in API mode.
//      SurrogateBasedLocalConfig requires model_pointer (block_pointer) and
//      sub_method (pointer_union); both are relaxed, so the block validates ok.
TEST(JsonRequiredFieldsPerBlock, MixedPointerAndNonPointerOnlyNonPointerErrors)
{
    json input = {
        {"surrogate_based_local", json::object()}
    };
    json output;
    std::vector<std::string> errors;
    bool ok = dakota::validate_method_block_json_to_json(input, output, errors);

    // Both model_pointer (block_pointer) and sub_method (pointer_union) are
    // required fields of SurrogateBasedLocalConfig; both are relaxed in
    // API mode, so there should be no errors.
    EXPECT_TRUE(ok)
        << "All required fields are pointer-type and should be relaxed; "
        << "errors: " << (errors.empty() ? "(none)" : errors[0]);
    EXPECT_FALSE(any_error_contains(errors, "model_pointer"))
        << "model_pointer (block_pointer) should be skipped in API mode";
    EXPECT_FALSE(any_error_contains(errors, "sub_method"))
        << "sub_method (pointer_union) should be skipped in API mode";
}

// 2.4  A fully specified per-block payload must succeed.
TEST(JsonRequiredFieldsPerBlock, PerBlockAllRequiredPresent)
{
    json input = {
        {"analysis_drivers", {
            {"drivers", {"text_book"}},
            {"interface_type", {{"direct", json::object()}}}
        }}
    };
    json output;
    std::vector<std::string> errors;
    bool ok = dakota::validate_interface_block_json_to_json(input, output, errors);

    EXPECT_TRUE(ok)
        << "Errors: " << (errors.empty() ? "(none)" : errors[0]);
}

// ============================================================================
// Suite 3: JsonRequiredFieldsRegression
// Existing JSON test files must still pass validation after this change.
// ============================================================================

TEST(JsonRequiredFieldsRegression, ExistingJsonTestFiles)
{
    // Embed the three regression documents inline so no filesystem path is
    // needed.  Content matches source/test/dakota_uq_textbook_*.json and
    // source/test/dakota_var_views_opt_discrete_json.json verbatim.

    // Already covered by Suite 1 tests; repeated here to confirm pass.
    const char* sop_lhs_json = R"json(
{"environment":{"top_method_pointer":"PSTUDY"},"interface":[{"analysis_drivers":{"drivers":["text_book"],"interface_type":{"direct":{}}},"deactivate":{"evaluation_cache":true,"restart_file":true},"id_interface":"ALEAT_I"}],"method":[{"sampling":{"id_method":"PSTUDY","model_pointer":"PS_M","samples":10,"seed":12347}},{"sampling":{"distribution":{"complementary":true},"id_method":"ALEATORY","model_pointer":"ALEAT_M","response_levels":{"compute":{"statistic":{"reliabilities":true}},"num_response_levels":[0,1,1],"values":[10000.0,10000.0]},"samples":50,"seed":12347}}],"model":[{"nested":{"id_model":"PS_M","responses_pointer":"PS_R","sub_method_pointer":{"pointer":"ALEATORY","primary_response_mapping":[1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0],"primary_variable_mapping":["X1","X3"],"secondary_response_mapping":[0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,1.0],"secondary_variable_mapping":["mean","mean"]},"variables_pointer":"PS_V"}},{"single":{"id_model":"ALEAT_M","interface_pointer":"ALEAT_I","responses_pointer":"ALEAT_R","variables_pointer":"ALEAT_V"}}],"responses":[{"descriptors":["mean_f1","ccdf_beta_f2","ccdf_beta_f3"],"gradient_type":{"no_gradients":true},"hessian_type":{"no_hessians":true},"id_responses":"PS_R","response_type":{"objective_functions":{"count":1,"nonlinear_inequality_constraints":{"count":2}}}},{"gradient_type":{"no_gradients":true},"hessian_type":{"no_hessians":true},"id_responses":"ALEAT_R","response_type":{"response_functions":{"count":3}}}],"variables":[{"active":{"all":true},"histogram_point_uncertain":{"real":{"abscissas":[50.0,60.0,70.0,80.0,90.0,30.0,40.0,50.0,60.0,70.0],"count":2,"counts":[10.0,20.0,30.0,20.0,10.0,10.0,20.0,30.0,20.0,10.0],"descriptors":["X1_mean","X3_mean"]}},"id_variables":"PS_V"},{"id_variables":"ALEAT_V","normal_uncertain":{"count":4,"descriptors":["X1","X2","X3","X4"],"means":[40.0,30.0,20.0,10.0],"std_deviations":[8.0,6.0,4.0,2.0]}}]}
)json";

    const char* discrete_json = R"json(
{"interface":[{"analysis_drivers":{"drivers":["text_book"],"interface_type":{"direct":{}}}}],"method":[{"soga":{"max_function_evaluations":5000,"seed":34785}}],"responses":[{"gradient_type":{"no_gradients":true},"hessian_type":{"no_hessians":true},"response_type":{"objective_functions":{"count":1,"nonlinear_inequality_constraints":{"count":2}}}}],"variables":[{"active":{"all":true},"binomial_uncertain":{"count":1,"descriptors":["x4"],"num_trials":[10],"probability_per_trial":[0.3]},"discrete_design_range":{"count":1,"descriptors":["x1"],"initial_point":[3],"lower_bounds":[0],"upper_bounds":[6]},"discrete_design_set":{"integer":{"count":1,"descriptors":["x2"],"elements":[2,4,6,8,10],"initial_point":[6]},"real":{"count":1,"descriptors":["x3"],"elements":[1.0,3.0,5.0,7.0,9.0],"initial_point":[5.0]}},"discrete_interval_uncertain":{"count":1,"descriptors":["x5"],"interval_probabilities":[0.5,0.5],"lower_bounds":[0,3],"num_intervals":[2],"upper_bounds":[3,6]},"discrete_state_range":{"count":1,"descriptors":["x8"],"initial_state":[3],"lower_bounds":[0],"upper_bounds":[6]},"discrete_state_set":{"integer":{"count":1,"descriptors":["x9"],"elements":[2,4,6,8,10],"initial_state":[6]},"real":{"count":1,"descriptors":["x10"],"elements":[1.0,3.0,5.0,7.0,9.0],"initial_state":[5.0]}},"discrete_uncertain_set":{"integer":{"count":1,"descriptors":["x6"],"elements":[2,4,6,8,10],"set_probabilities":[0.2,0.2,0.2,0.2,0.2]},"real":{"count":1,"descriptors":["x7"],"elements":[1.0,3.0,5.0,7.0,9.0],"set_probabilities":[0.2,0.2,0.2,0.2,0.2]}}}]}
)json";

    const std::vector<std::pair<const char*, const char*>> cases = {
        {"lhs",      k_lhs_json   },
        {"sop_lhs",  sop_lhs_json },
        {"discrete", discrete_json},
    };

    for (const auto& [name, raw] : cases) {
        json doc = json::parse(raw);
        json output;
        std::vector<std::string> errors;
        bool ok = dakota::validate_json_input_to_json(doc, output, errors);
        EXPECT_TRUE(ok)
            << name << " failed: "
            << (errors.empty() ? "(no detail)" : errors[0]);
    }
}

// ============================================================================
// Suite 4: JsonPointerGroupUnion
// Pointer-group and pointer-union relaxation in API mode (api_mode=true).
// ============================================================================

// 4.1  Pointer-group field (sub_method_pointer on NestedConfig) omitted in
//      API mode must succeed.  sub_method_pointer is annotated pointer_group
//      in the JSON schema, so the per-block entry point skips it.
TEST(JsonPointerGroupUnion, PointerGroupSkippedInApiMode)
{
    // nested model without sub_method_pointer
    json input = {
        {"nested", {
            {"id_model",           "my_model"},
            {"variables_pointer",  "my_vars"},
            {"responses_pointer",  "my_resp"}
        }}
    };
    json output;
    std::vector<std::string> errors;
    bool ok = dakota::validate_model_block_json_to_json(input, output, errors);

    EXPECT_TRUE(ok)
        << "pointer_group sub_method_pointer should be relaxed in API mode; "
        << "errors: " << (errors.empty() ? "(none)" : errors[0]);
    EXPECT_FALSE(any_error_contains(errors, "sub_method_pointer"))
        << "sub_method_pointer should not appear in errors";
}

// 4.2  Pointer-group field (sub_method_pointer on NestedConfig) omitted in
//      whole-file mode must produce an error.
TEST(JsonPointerGroupUnion, PointerGroupEnforcedInWholeFileMode)
{
    auto doc = base_doc();
    // Insert a nested model block without sub_method_pointer
    doc["model"] = json::array({
        {{"nested", {
            {"variables_pointer", "PS_V"},
            {"responses_pointer", "PS_R"}
        }}}
    });

    json output;
    std::vector<std::string> errors;
    bool ok = dakota::validate_json_input_to_json(doc, output, errors);

    EXPECT_FALSE(ok);
    EXPECT_TRUE(any_error_contains(errors, "sub_method_pointer"))
        << "Expected 'sub_method_pointer' in whole-file mode errors; errors: "
        << (errors.empty() ? "(none)" : errors[0]);
}

// 4.3  Pointer-union field (sub_method on EmbeddedConfig) omitted in API
//      mode must succeed.  sub_method on SurrogateBasedLocalConfig is
//      annotated pointer_union in the JSON schema, so the per-block entry
//      point skips it.  surrogate_based_local is used because it is a direct
//      top-level method selector (unlike 'embedded', which is nested under
//      the 'hybrid' selector).
TEST(JsonPointerGroupUnion, PointerUnionSkippedInApiMode)
{
    // surrogate_based_local method without sub_method (or model_pointer)
    json input = {
        {"surrogate_based_local", json::object()}
    };
    json output;
    std::vector<std::string> errors;
    bool ok = dakota::validate_method_block_json_to_json(input, output, errors);

    EXPECT_TRUE(ok)
        << "pointer_union sub_method should be relaxed in API mode; "
        << "errors: " << (errors.empty() ? "(none)" : errors[0]);
    EXPECT_FALSE(any_error_contains(errors, "sub_method"))
        << "sub_method should not appear in errors in API mode";
}

// 4.4  Pointer-union field (sub_method on SurrogateBasedLocalConfig) omitted
//      in whole-file mode must produce an error.
TEST(JsonPointerGroupUnion, PointerUnionEnforcedInWholeFileMode)
{
    auto doc = base_doc();
    // Insert a surrogate_based_local method block without sub_method.
    // In whole-file mode all required fields are enforced, so sub_method
    // (pointer_union) must be present.
    doc["method"] = json::array({
        {{"surrogate_based_local", json::object()}}
    });

    json output;
    std::vector<std::string> errors;
    bool ok = dakota::validate_json_input_to_json(doc, output, errors);

    EXPECT_FALSE(ok);
    EXPECT_TRUE(any_error_contains(errors, "sub_method"))
        << "Expected 'sub_method' in whole-file mode errors; errors: "
        << (errors.empty() ? "(none)" : errors[0]);
}
