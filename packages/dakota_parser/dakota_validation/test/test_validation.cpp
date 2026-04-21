// test_validation.cpp
// C++ unit tests for validation functions

#include <gtest/gtest.h>
#include <dakota/validation.hpp>
#include <dakota/variable_validators.hpp>
#include <dakota/computed_fields.hpp>
#include <cmath>
#include <limits>

using namespace dakota::validation;
using namespace dakota::computed_fields;
using json = nlohmann::json;

// ============================================================================
// CompareLength tests
// ============================================================================

TEST(CompareLength, PassesWhenLengthMatches) {
    json instance = {
        {"count", 3},
        {"values", {1.0, 2.0, 3.0}}
    };
    
    json mutations = compare_length(
        instance, 
        {"values", "count"},  // fields
        json::array(),        // literals (empty)
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CompareLength, SkipsNullField) {
    json instance = {
        {"count", 3},
        {"values", nullptr}
    };
    
    json mutations = compare_length(
        instance,
        {"values", "count"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CompareLength, SkipsMissingField) {
    json instance = {
        {"count", 3}
    };
    
    json mutations = compare_length(
        instance,
        {"values", "count"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CompareLength, ThrowsOnLengthMismatch) {
    json instance = {
        {"count", 3},
        {"values", {1.0, 2.0}}
    };
    
    EXPECT_THROW(
        compare_length(instance, {"values", "count"}, json::array(), "test"),
        ValidationError
    );
}

TEST(CompareLength, ErrorMessageContainsSizes) {
    json instance = {
        {"count", 5},
        {"values", {1.0, 2.0}}
    };
    
    try {
        compare_length(instance, {"values", "count"}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("2"), std::string::npos);
        EXPECT_NE(msg.find("5"), std::string::npos);
    }
}

TEST(CompareLength, SupportsDottedPathForListField) {
    json instance = {
        {"count", 3},
        {"child", {
            {"values", {1.0, 2.0, 3.0}}
        }}
    };
    
    json mutations = compare_length(
        instance, 
        {"child.values", "count"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CompareLength, SupportsDottedPathForTargetField) {
    json instance = {
        {"values", {1.0, 2.0, 3.0}},
        {"child", {
            {"count", 3}
        }}
    };
    
    json mutations = compare_length(
        instance, 
        {"values", "child.count"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CompareLength, SupportsBothFieldsDotted) {
    json instance = {
        {"parent", {
            {"values", {1.0, 2.0}}
        }},
        {"config", {
            {"size", 2}
        }}
    };
    
    json mutations = compare_length(
        instance, 
        {"parent.values", "config.size"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CompareLength, ThrowsOnDottedPathMismatch) {
    json instance = {
        {"child", {
            {"values", {1.0, 2.0}}
        }},
        {"count", 5}
    };
    
    EXPECT_THROW(
        compare_length(instance, {"child.values", "count"}, json::array(), "test"),
        ValidationError
    );
}

TEST(CompareLength, SkipsNullNestedField) {
    json instance = {
        {"count", 3},
        {"child", {
            {"values", nullptr}
        }}
    };
    
    json mutations = compare_length(
        instance,
        {"child.values", "count"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CompareLength, SkipsMissingNestedField) {
    json instance = {
        {"count", 3},
        {"child", {
            {"other", 42}
        }}
    };
    
    json mutations = compare_length(
        instance,
        {"child.values", "count"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(VariableValidators, NormalUncertainInitialRejectsInternalFlagInput) {
    json instance = {
        {"count", 1},
        {"means", {1.0}},
        {"std_deviations", {0.5}},
        {"initial_point_user_provided", true}
    };

    EXPECT_THROW(
        normal_uncertain_initial(instance, {}, json::array(), "normal_uncertain"),
        ValidationError
    );
}

TEST(VariableValidators, NormalUncertainBoundsRejectsInferredBoundsInput) {
    json instance = {
        {"count", 1},
        {"means", {1.0}},
        {"std_deviations", {0.5}},
        {"inferred_lower_bounds", {0.0}}
    };

    EXPECT_THROW(
        normal_uncertain_bounds(instance, {}, json::array(), "normal_uncertain"),
        ValidationError
    );
}

TEST(VariableValidators, LognormalUncertainBoundsRejectsInferredBoundsInput) {
    json instance = {
        {"count", 1},
        {"parameters", {
            {"option_1", {
                {"means", {1.0}},
                {"std_deviations", {0.5}}
            }}
        }},
        {"inferred_upper_bounds", {2.0}}
    };

    EXPECT_THROW(
        lognormal_uncertain_bounds(instance, {}, json::array(), "lognormal_uncertain"),
        ValidationError
    );
}

// ============================================================================
// CompareLengthOne tests
// ============================================================================

TEST(CompareLengthOne, PassesWhenLengthMatches) {
    json instance = {
        {"count", 3},
        {"scale", {1.0, 2.0, 3.0}}
    };
    
    json mutations = compare_length_one(
        instance,
        {"scale", "count"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CompareLengthOne, ExpandsSingleValue) {
    json instance = {
        {"count", 3},
        {"scale", {1.5}}
    };
    
    json mutations = compare_length_one(
        instance,
        {"scale", "count"},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("scale"));
    auto expanded = mutations["scale"];
    ASSERT_EQ(expanded.size(), 3);
    EXPECT_DOUBLE_EQ(expanded[0].get<double>(), 1.5);
    EXPECT_DOUBLE_EQ(expanded[1].get<double>(), 1.5);
    EXPECT_DOUBLE_EQ(expanded[2].get<double>(), 1.5);
}

TEST(CompareLengthOne, ExpandsStringList) {
    json instance = {
        {"count", 3},
        {"labels", {"auto"}}
    };
    
    json mutations = compare_length_one(
        instance,
        {"labels", "count"},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("labels"));
    auto expanded = mutations["labels"];
    ASSERT_EQ(expanded.size(), 3);
    EXPECT_EQ(expanded[0].get<std::string>(), "auto");
    EXPECT_EQ(expanded[1].get<std::string>(), "auto");
    EXPECT_EQ(expanded[2].get<std::string>(), "auto");
}

TEST(CompareLengthOne, ThrowsOnInvalidLength) {
    json instance = {
        {"count", 3},
        {"scale", {1.0, 2.0}}
    };
    
    EXPECT_THROW(
        compare_length_one(instance, {"scale", "count"}, json::array(), "test"),
        ValidationError
    );
}

TEST(CompareLengthOne, SupportsDottedPathForTargetField) {
    json instance = {
        {"scale", {1.0, 2.0, 3.0}},
        {"child", {
            {"count", 3}
        }}
    };
    
    json mutations = compare_length_one(
        instance, 
        {"scale", "child.count"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CompareLengthOne, ExpandsUsingDottedTarget) {
    json instance = {
        {"scale", {1.5}},
        {"config", {
            {"size", 3}
        }}
    };
    
    json mutations = compare_length_one(
        instance,
        {"scale", "config.size"},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("scale"));
    auto expanded = mutations["scale"];
    ASSERT_EQ(expanded.size(), 3);
}

// ============================================================================
// CheckNonnegativeList tests
// ============================================================================

TEST(CheckNonnegativeList, PassesAllPositive) {
    json instance = {
        {"values", {1.0, 2.0, 3.0}}
    };
    
    json mutations = check_nonnegative_list(
        instance,
        {"values"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckNonnegativeList, PassesWithZero) {
    json instance = {
        {"values", {0.0, 1.0, 0.0}}
    };
    
    json mutations = check_nonnegative_list(
        instance,
        {"values"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckNonnegativeList, SkipsNullField) {
    json instance = {
        {"values", nullptr}
    };
    
    json mutations = check_nonnegative_list(
        instance,
        {"values"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckNonnegativeList, SkipsMissingField) {
    json instance = {
        {"other", 42}
    };
    
    json mutations = check_nonnegative_list(
        instance,
        {"values"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckNonnegativeList, ThrowsOnNegative) {
    json instance = {
        {"values", {1.0, -2.0, 3.0}}
    };
    
    EXPECT_THROW(
        check_nonnegative_list(instance, {"values"}, json::array(), "test"),
        ValidationError
    );
}

TEST(CheckNonnegativeList, ErrorMessageContainsIndex) {
    json instance = {
        {"values", {1.0, -2.0, 3.0}}
    };
    
    try {
        check_nonnegative_list(instance, {"values"}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("element 1"), std::string::npos);
        EXPECT_NE(msg.find("negative"), std::string::npos);
    }
}

TEST(CheckNonnegativeList, WorksWithIntegers) {
    json instance = {
        {"values", {1, -2, 3}}
    };
    
    EXPECT_THROW(
        check_nonnegative_list(instance, {"values"}, json::array(), "test"),
        ValidationError
    );
}

// ============================================================================
// CheckPositiveList tests
// ============================================================================

TEST(CheckPositiveList, PassesAllPositive) {
    json instance = {
        {"values", {1, 2, 3}}
    };
    
    json mutations = check_positive_list(
        instance,
        {"values"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckPositiveList, SkipsNullField) {
    json instance = {
        {"values", nullptr}
    };
    
    json mutations = check_positive_list(
        instance,
        {"values"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckPositiveList, ThrowsOnZero) {
    json instance = {
        {"values", {1, 0, 3}}
    };
    
    EXPECT_THROW(
        check_positive_list(instance, {"values"}, json::array(), "test"),
        ValidationError
    );
}

TEST(CheckPositiveList, ThrowsOnNegative) {
    json instance = {
        {"values", {1, -2, 3}}
    };
    
    EXPECT_THROW(
        check_positive_list(instance, {"values"}, json::array(), "test"),
        ValidationError
    );
}

TEST(CheckPositiveList, ErrorMessageContainsIndex) {
    json instance = {
        {"values", {1, 0, 3}}
    };
    
    try {
        check_positive_list(instance, {"values"}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("element 1"), std::string::npos);
        EXPECT_NE(msg.find("not positive"), std::string::npos);
    }
}

// ============================================================================
// CheckProbabilityList tests
// ============================================================================

TEST(CheckProbabilityList, PassesValidProbabilities) {
    json instance = {
        {"probs", {0.0, 0.5, 1.0}}
    };
    
    json mutations = check_probability_list(
        instance,
        {"probs"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckProbabilityList, SkipsNullField) {
    json instance = {
        {"probs", nullptr}
    };
    
    json mutations = check_probability_list(
        instance,
        {"probs"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckProbabilityList, ThrowsOnNegative) {
    json instance = {
        {"probs", {0.5, -0.1, 0.8}}
    };
    
    EXPECT_THROW(
        check_probability_list(instance, {"probs"}, json::array(), "test"),
        ValidationError
    );
}

TEST(CheckProbabilityList, ThrowsOnGreaterThanOne) {
    json instance = {
        {"probs", {0.5, 1.1, 0.8}}
    };
    
    EXPECT_THROW(
        check_probability_list(instance, {"probs"}, json::array(), "test"),
        ValidationError
    );
}

TEST(CheckProbabilityList, ErrorMessageContainsValue) {
    json instance = {
        {"probs", {0.5, 1.5, 0.8}}
    };
    
    try {
        check_probability_list(instance, {"probs"}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("element 1"), std::string::npos);
        EXPECT_NE(msg.find("not in [0, 1]"), std::string::npos);
    }
}

// ============================================================================
// CheckRealLowerBound tests
// ============================================================================

TEST(CheckRealLowerBound, PassesAboveBound) {
    json instance = {
        {"values", {0.1, 1.0, 5.0}}
    };
    
    json mutations = check_real_lower_bound(
        instance,
        {"values"},
        json::array({0.0}),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckRealLowerBound, SkipsNullField) {
    json instance = {
        {"values", nullptr}
    };
    
    json mutations = check_real_lower_bound(
        instance,
        {"values"},
        json::array({0.0}),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckRealLowerBound, ThrowsOnEqualToBound) {
    json instance = {
        {"values", {1.0, 0.0, 2.0}}
    };
    
    EXPECT_THROW(
        check_real_lower_bound(instance, {"values"}, json::array({0.0}), "test"),
        ValidationError
    );
}

TEST(CheckRealLowerBound, ThrowsOnBelowBound) {
    json instance = {
        {"values", {1.0, -0.5, 2.0}}
    };
    
    EXPECT_THROW(
        check_real_lower_bound(instance, {"values"}, json::array({0.0}), "test"),
        ValidationError
    );
}

TEST(CheckRealLowerBound, ErrorMessageContainsValueAndBound) {
    json instance = {
        {"values", {1.0, -0.5, 2.0}}
    };
    
    try {
        check_real_lower_bound(instance, {"values"}, json::array({0.0}), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("element 1"), std::string::npos);
        EXPECT_NE(msg.find("not >"), std::string::npos);
    }
}

TEST(CheckRealLowerBound, InclusivePassesOnEqual) {
    json instance = {
        {"values", {0.0, 1.0, 5.0}}
    };
    
    json mutations = check_real_lower_bound(
        instance,
        {"values"},
        json::array({0.0, true}),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckRealLowerBound, InclusiveThrowsOnBelow) {
    json instance = {
        {"values", {1.0, -0.1, 2.0}}
    };
    
    EXPECT_THROW(
        check_real_lower_bound(instance, {"values"}, json::array({0.0, true}), "test"),
        ValidationError
    );
}

TEST(CheckRealLowerBound, InclusiveErrorMessage) {
    json instance = {
        {"values", {1.0, -0.1, 2.0}}
    };
    
    try {
        check_real_lower_bound(instance, {"values"}, json::array({0.0, true}), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("element 1"), std::string::npos);
        EXPECT_NE(msg.find("not >="), std::string::npos);
    }
}

// ============================================================================
// CheckRealUpperBound tests
// ============================================================================

TEST(CheckRealUpperBound, PassesBelowBound) {
    json instance = {
        {"values", {0.1, 0.5, 0.9}}
    };
    
    json mutations = check_real_upper_bound(
        instance,
        {"values"},
        json::array({1.0}),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckRealUpperBound, SkipsNullField) {
    json instance = {
        {"values", nullptr}
    };
    
    json mutations = check_real_upper_bound(
        instance,
        {"values"},
        json::array({1.0}),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckRealUpperBound, ThrowsOnEqualToBound) {
    json instance = {
        {"values", {0.5, 1.0, 0.8}}
    };
    
    EXPECT_THROW(
        check_real_upper_bound(instance, {"values"}, json::array({1.0}), "test"),
        ValidationError
    );
}

TEST(CheckRealUpperBound, ThrowsOnAboveBound) {
    json instance = {
        {"values", {0.5, 1.5, 0.8}}
    };
    
    EXPECT_THROW(
        check_real_upper_bound(instance, {"values"}, json::array({1.0}), "test"),
        ValidationError
    );
}

TEST(CheckRealUpperBound, InclusivePassesOnEqual) {
    json instance = {
        {"values", {0.5, 1.0, 0.8}}
    };
    
    json mutations = check_real_upper_bound(
        instance,
        {"values"},
        json::array({1.0, true}),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckRealUpperBound, InclusiveThrowsOnAbove) {
    json instance = {
        {"values", {0.5, 1.1, 0.8}}
    };
    
    EXPECT_THROW(
        check_real_upper_bound(instance, {"values"}, json::array({1.0, true}), "test"),
        ValidationError
    );
}

TEST(CheckRealUpperBound, InclusiveErrorMessage) {
    json instance = {
        {"values", {0.5, 1.1, 0.8}}
    };
    
    try {
        check_real_upper_bound(instance, {"values"}, json::array({1.0, true}), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("element 1"), std::string::npos);
        EXPECT_NE(msg.find("not <="), std::string::npos);
    }
}

// ============================================================================
// CheckIntLowerBound tests
// ============================================================================

TEST(CheckIntLowerBound, PassesAboveBound) {
    json instance = {
        {"values", {1, 2, 3}}
    };
    
    json mutations = check_int_lower_bound(
        instance,
        {"values"},
        json::array({0}),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckIntLowerBound, SkipsNullField) {
    json instance = {
        {"values", nullptr}
    };
    
    json mutations = check_int_lower_bound(
        instance,
        {"values"},
        json::array({0}),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckIntLowerBound, ThrowsOnEqualToBound) {
    json instance = {
        {"values", {1, 0, 3}}
    };
    
    EXPECT_THROW(
        check_int_lower_bound(instance, {"values"}, json::array({0}), "test"),
        ValidationError
    );
}

TEST(CheckIntLowerBound, ThrowsOnBelowBound) {
    json instance = {
        {"values", {1, -5, 3}}
    };
    
    EXPECT_THROW(
        check_int_lower_bound(instance, {"values"}, json::array({0}), "test"),
        ValidationError
    );
}

// ============================================================================
// CheckSumEqualsLength tests
// ============================================================================

TEST(CheckSumEqualsLength, PassesWhenSumMatchesLength) {
    json instance = {
        {"num_list", {2, 3}},       // sum = 5
        {"levels_list", {0.1, 0.2, 0.3, 0.4, 0.5}}  // length = 5
    };
    
    json mutations = check_sum_equals_length(
        instance,
        {"num_list", "levels_list"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckSumEqualsLength, SkipsNullNumList) {
    json instance = {
        {"num_list", nullptr},
        {"levels_list", {0.1, 0.2, 0.3}}
    };
    
    json mutations = check_sum_equals_length(
        instance,
        {"num_list", "levels_list"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckSumEqualsLength, SkipsNullLevelsList) {
    json instance = {
        {"num_list", {2, 3}},
        {"levels_list", nullptr}
    };
    
    json mutations = check_sum_equals_length(
        instance,
        {"num_list", "levels_list"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckSumEqualsLength, ThrowsOnMismatch) {
    json instance = {
        {"num_list", {2, 3}},       // sum = 5
        {"levels_list", {0.1, 0.2, 0.3}}  // length = 3
    };
    
    EXPECT_THROW(
        check_sum_equals_length(instance, {"num_list", "levels_list"}, json::array(), "test"),
        ValidationError
    );
}

TEST(CheckSumEqualsLength, ErrorMessageContainsSumAndLength) {
    json instance = {
        {"num_list", {2, 3}},       // sum = 5
        {"levels_list", {0.1, 0.2, 0.3}}  // length = 3
    };
    
    try {
        check_sum_equals_length(instance, {"num_list", "levels_list"}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("5"), std::string::npos);
        EXPECT_NE(msg.find("3"), std::string::npos);
    }
}

// ============================================================================
// TrustRegionValidate tests
// ============================================================================

TEST(TrustRegionValidate, PassesValidParams) {
    json instance = {
        {"initial_size", {0.5, 0.6}},
        {"minimum_size", 0.1},
        {"contract_threshold", 0.25},
        {"expand_threshold", 0.75},
        {"contraction_factor", 0.5},
        {"expansion_factor", 2.0}
    };
    
    json mutations = trust_region_validate(
        instance,
        {"initial_size", "minimum_size", "contract_threshold", 
         "expand_threshold", "contraction_factor", "expansion_factor"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(TrustRegionValidate, PassesMinimalParams) {
    json instance = {
        {"initial_size", nullptr},
        {"minimum_size", nullptr},
        {"contract_threshold", 0.25},
        {"expand_threshold", 0.75},
        {"contraction_factor", nullptr},
        {"expansion_factor", nullptr}
    };
    
    json mutations = trust_region_validate(
        instance,
        {"initial_size", "minimum_size", "contract_threshold", 
         "expand_threshold", "contraction_factor", "expansion_factor"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(TrustRegionValidate, ThrowsInitialSizeZero) {
    json instance = {
        {"initial_size", {0.5, 0.0}},
        {"minimum_size", nullptr},
        {"contract_threshold", 0.25},
        {"expand_threshold", 0.75},
        {"contraction_factor", nullptr},
        {"expansion_factor", nullptr}
    };
    
    EXPECT_THROW(
        trust_region_validate(
            instance,
            {"initial_size", "minimum_size", "contract_threshold", 
             "expand_threshold", "contraction_factor", "expansion_factor"},
            json::array(),
            "test"
        ),
        ValidationError
    );
}

TEST(TrustRegionValidate, ThrowsInitialSizeGreaterThanOne) {
    json instance = {
        {"initial_size", {0.5, 1.5}},
        {"minimum_size", nullptr},
        {"contract_threshold", 0.25},
        {"expand_threshold", 0.75},
        {"contraction_factor", nullptr},
        {"expansion_factor", nullptr}
    };
    
    EXPECT_THROW(
        trust_region_validate(
            instance,
            {"initial_size", "minimum_size", "contract_threshold", 
             "expand_threshold", "contraction_factor", "expansion_factor"},
            json::array(),
            "test"
        ),
        ValidationError
    );
}

TEST(TrustRegionValidate, ThrowsInitialSizeLessThanMinimum) {
    json instance = {
        {"initial_size", {0.5, 0.2}},
        {"minimum_size", 0.3},  // 0.2 < 0.3
        {"contract_threshold", 0.25},
        {"expand_threshold", 0.75},
        {"contraction_factor", nullptr},
        {"expansion_factor", nullptr}
    };
    
    EXPECT_THROW(
        trust_region_validate(
            instance,
            {"initial_size", "minimum_size", "contract_threshold", 
             "expand_threshold", "contraction_factor", "expansion_factor"},
            json::array(),
            "test"
        ),
        ValidationError
    );
}

TEST(TrustRegionValidate, ThrowsMinimumSizeNegative) {
    json instance = {
        {"initial_size", nullptr},
        {"minimum_size", -0.1},
        {"contract_threshold", 0.25},
        {"expand_threshold", 0.75},
        {"contraction_factor", nullptr},
        {"expansion_factor", nullptr}
    };
    
    EXPECT_THROW(
        trust_region_validate(
            instance,
            {"initial_size", "minimum_size", "contract_threshold", 
             "expand_threshold", "contraction_factor", "expansion_factor"},
            json::array(),
            "test"
        ),
        ValidationError
    );
}

TEST(TrustRegionValidate, ThrowsContractThresholdZero) {
    json instance = {
        {"initial_size", nullptr},
        {"minimum_size", nullptr},
        {"contract_threshold", 0.0},
        {"expand_threshold", 0.75},
        {"contraction_factor", nullptr},
        {"expansion_factor", nullptr}
    };
    
    EXPECT_THROW(
        trust_region_validate(
            instance,
            {"initial_size", "minimum_size", "contract_threshold", 
             "expand_threshold", "contraction_factor", "expansion_factor"},
            json::array(),
            "test"
        ),
        ValidationError
    );
}

TEST(TrustRegionValidate, ThrowsContractGreaterThanExpand) {
    json instance = {
        {"initial_size", nullptr},
        {"minimum_size", nullptr},
        {"contract_threshold", 0.8},
        {"expand_threshold", 0.5},  // 0.8 > 0.5
        {"contraction_factor", nullptr},
        {"expansion_factor", nullptr}
    };
    
    EXPECT_THROW(
        trust_region_validate(
            instance,
            {"initial_size", "minimum_size", "contract_threshold", 
             "expand_threshold", "contraction_factor", "expansion_factor"},
            json::array(),
            "test"
        ),
        ValidationError
    );
}

TEST(TrustRegionValidate, ThrowsExpandThresholdGreaterThanOne) {
    json instance = {
        {"initial_size", nullptr},
        {"minimum_size", nullptr},
        {"contract_threshold", 0.25},
        {"expand_threshold", 1.5},
        {"contraction_factor", nullptr},
        {"expansion_factor", nullptr}
    };
    
    EXPECT_THROW(
        trust_region_validate(
            instance,
            {"initial_size", "minimum_size", "contract_threshold", 
             "expand_threshold", "contraction_factor", "expansion_factor"},
            json::array(),
            "test"
        ),
        ValidationError
    );
}

TEST(TrustRegionValidate, ThrowsContractionFactorZero) {
    json instance = {
        {"initial_size", nullptr},
        {"minimum_size", nullptr},
        {"contract_threshold", 0.25},
        {"expand_threshold", 0.75},
        {"contraction_factor", 0.0},
        {"expansion_factor", nullptr}
    };
    
    EXPECT_THROW(
        trust_region_validate(
            instance,
            {"initial_size", "minimum_size", "contract_threshold", 
             "expand_threshold", "contraction_factor", "expansion_factor"},
            json::array(),
            "test"
        ),
        ValidationError
    );
}

TEST(TrustRegionValidate, ThrowsContractionFactorGreaterThanOne) {
    json instance = {
        {"initial_size", nullptr},
        {"minimum_size", nullptr},
        {"contract_threshold", 0.25},
        {"expand_threshold", 0.75},
        {"contraction_factor", 1.5},
        {"expansion_factor", nullptr}
    };
    
    EXPECT_THROW(
        trust_region_validate(
            instance,
            {"initial_size", "minimum_size", "contract_threshold", 
             "expand_threshold", "contraction_factor", "expansion_factor"},
            json::array(),
            "test"
        ),
        ValidationError
    );
}

TEST(TrustRegionValidate, ThrowsExpansionFactorLessThanOne) {
    json instance = {
        {"initial_size", nullptr},
        {"minimum_size", nullptr},
        {"contract_threshold", 0.25},
        {"expand_threshold", 0.75},
        {"contraction_factor", nullptr},
        {"expansion_factor", 0.5}
    };
    
    EXPECT_THROW(
        trust_region_validate(
            instance,
            {"initial_size", "minimum_size", "contract_threshold", 
             "expand_threshold", "contraction_factor", "expansion_factor"},
            json::array(),
            "test"
        ),
        ValidationError
    );
}

TEST(TrustRegionValidate, ContractionFactorOneIsValid) {
    // contraction_factor = 1.0 is valid (Dakota warns but doesn't error)
    json instance = {
        {"initial_size", nullptr},
        {"minimum_size", nullptr},
        {"contract_threshold", 0.25},
        {"expand_threshold", 0.75},
        {"contraction_factor", 1.0},
        {"expansion_factor", nullptr}
    };
    
    json mutations = trust_region_validate(
        instance,
        {"initial_size", "minimum_size", "contract_threshold", 
         "expand_threshold", "contraction_factor", "expansion_factor"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(TrustRegionValidate, ErrorMessageContainsFieldName) {
    json instance = {
        {"initial_size", {0.5, 0.0}},
        {"minimum_size", nullptr},
        {"contract_threshold", 0.25},
        {"expand_threshold", 0.75},
        {"contraction_factor", nullptr},
        {"expansion_factor", nullptr}
    };
    
    try {
        trust_region_validate(
            instance,
            {"initial_size", "minimum_size", "contract_threshold", 
             "expand_threshold", "contraction_factor", "expansion_factor"},
            json::array(),
            "test"
        );
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("initial_size"), std::string::npos);
        EXPECT_NE(msg.find("[1]"), std::string::npos);
    }
}

// ============================================================================
// ValidatorRegistry tests for new validators
// ============================================================================

TEST(ValidatorRegistry, HasNewValidators) {
    auto& registry = ValidatorRegistry::instance();
    
    // List element validators
    EXPECT_TRUE(registry.has_validator("check_nonnegative_list"));
    EXPECT_TRUE(registry.has_validator("check_positive_list"));
    EXPECT_TRUE(registry.has_validator("check_probability_list"));
    EXPECT_TRUE(registry.has_validator("check_real_lower_bound"));
    EXPECT_TRUE(registry.has_validator("check_real_upper_bound"));
    EXPECT_TRUE(registry.has_validator("check_int_lower_bound"));
    
    // Cross-field validators
    EXPECT_TRUE(registry.has_validator("check_sum_equals_length"));
    EXPECT_TRUE(registry.has_validator("trust_region_validate"));
}

TEST(ValidatorRegistry, DispatchNewValidators) {
    auto& registry = ValidatorRegistry::instance();
    
    json instance = {
        {"values", {1.0, 2.0, 3.0}}
    };
    
    // Test dispatch to check_nonnegative_list
    json mutations = registry.validate(
        instance,
        "check_nonnegative_list",
        {"values"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

// ============================================================================
// DefaultBoundsReal tests
// ============================================================================

TEST(DefaultBoundsReal, SetsLowerWhenNull) {
    json instance = {
        {"count", 3},
        {"lower_bounds", nullptr},
        {"upper_bounds", {1.0, 2.0, 3.0}}
    };
    
    json mutations = default_bounds_real(
        instance,
        {},
        json::array({-1e10, 1e10}),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("lower_bounds"));
    EXPECT_EQ(mutations["lower_bounds"].size(), 3);
    EXPECT_DOUBLE_EQ(mutations["lower_bounds"][0].get<double>(), -1e10);
}

TEST(DefaultBoundsReal, SetsUpperWhenNull) {
    json instance = {
        {"count", 2},
        {"lower_bounds", {0.0, 0.0}},
        {"upper_bounds", nullptr}
    };
    
    json mutations = default_bounds_real(
        instance,
        {},
        json::array({-1e10, 1e10}),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("upper_bounds"));
    ASSERT_FALSE(mutations.contains("lower_bounds"));
    EXPECT_EQ(mutations["upper_bounds"].size(), 2);
    EXPECT_DOUBLE_EQ(mutations["upper_bounds"][0].get<double>(), 1e10);
}

TEST(DefaultBoundsReal, SetsBothWhenBothNull) {
    json instance = {
        {"count", 2},
        {"lower_bounds", nullptr},
        {"upper_bounds", nullptr}
    };
    
    json mutations = default_bounds_real(
        instance,
        {},
        json::array({-100.0, 100.0}),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("lower_bounds"));
    ASSERT_TRUE(mutations.contains("upper_bounds"));
    EXPECT_DOUBLE_EQ(mutations["lower_bounds"][0].get<double>(), -100.0);
    EXPECT_DOUBLE_EQ(mutations["upper_bounds"][0].get<double>(), 100.0);
}

TEST(DefaultBoundsReal, NoChangeWhenBothSet) {
    json instance = {
        {"count", 2},
        {"lower_bounds", {0.0, 1.0}},
        {"upper_bounds", {10.0, 20.0}}
    };
    
    json mutations = default_bounds_real(
        instance,
        {},
        json::array({-1e10, 1e10}),
        "test"
    );
    
    EXPECT_TRUE(mutations.empty());
}

// ============================================================================
// DefaultInitialPointReal tests
// ============================================================================

TEST(DefaultInitialPointReal, SetsMidpointWhenNull) {
    json instance = {
        {"count", 3},
        {"lower_bounds", {0.0, -10.0, 5.0}},
        {"upper_bounds", {10.0, 10.0, 15.0}},
        {"initial_point", nullptr}
    };
    
    json mutations = default_initial_point_real(
        instance,
        {"initial_point"},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("initial_point"));
    auto ip = mutations["initial_point"];
    EXPECT_DOUBLE_EQ(ip[0].get<double>(), 5.0);   // (0 + 10) / 2
    EXPECT_DOUBLE_EQ(ip[1].get<double>(), 0.0);   // (-10 + 10) / 2
    EXPECT_DOUBLE_EQ(ip[2].get<double>(), 10.0);  // (5 + 15) / 2
}

TEST(DefaultInitialPointReal, UsesFiniteBoundWhenOneInfinite) {
    double inf = std::numeric_limits<double>::infinity();
    json instance = {
        {"count", 2},
        {"lower_bounds", {-inf, 5.0}},
        {"upper_bounds", {10.0, inf}},
        {"initial_point", nullptr}
    };
    
    json mutations = default_initial_point_real(
        instance,
        {"initial_point"},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("initial_point"));
    auto ip = mutations["initial_point"];
    EXPECT_DOUBLE_EQ(ip[0].get<double>(), 10.0);  // Uses upper
    EXPECT_DOUBLE_EQ(ip[1].get<double>(), 5.0);   // Uses lower
}

TEST(DefaultInitialPointReal, UsesZeroWhenBothInfinite) {
    double inf = std::numeric_limits<double>::infinity();
    json instance = {
        {"count", 1},
        {"lower_bounds", {-inf}},
        {"upper_bounds", {inf}},
        {"initial_point", nullptr}
    };
    
    json mutations = default_initial_point_real(
        instance,
        {"initial_point"},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("initial_point"));
    EXPECT_DOUBLE_EQ(mutations["initial_point"][0].get<double>(), 0.0);
}

TEST(DefaultInitialPointReal, ClampsToLowerBound) {
    json instance = {
        {"count", 2},
        {"lower_bounds", {0.0, 5.0}},
        {"upper_bounds", {10.0, 15.0}},
        {"initial_point", {-5.0, 10.0}}  // -5 < lower[0]=0
    };
    
    json mutations = default_initial_point_real(
        instance,
        {"initial_point"},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("initial_point"));
    auto ip = mutations["initial_point"];
    EXPECT_DOUBLE_EQ(ip[0].get<double>(), 0.0);   // Clamped
    EXPECT_DOUBLE_EQ(ip[1].get<double>(), 10.0);  // Unchanged
}

TEST(DefaultInitialPointReal, ClampsToUpperBound) {
    json instance = {
        {"count", 2},
        {"lower_bounds", {0.0, 5.0}},
        {"upper_bounds", {10.0, 15.0}},
        {"initial_point", {5.0, 20.0}}  // 20 > upper[1]=15
    };
    
    json mutations = default_initial_point_real(
        instance,
        {"initial_point"},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("initial_point"));
    auto ip = mutations["initial_point"];
    EXPECT_DOUBLE_EQ(ip[0].get<double>(), 5.0);   // Unchanged
    EXPECT_DOUBLE_EQ(ip[1].get<double>(), 15.0);  // Clamped
}

TEST(DefaultInitialPointReal, NoChangeWhenWithinBounds) {
    json instance = {
        {"count", 2},
        {"lower_bounds", {0.0, 0.0}},
        {"upper_bounds", {10.0, 10.0}},
        {"initial_point", {5.0, 5.0}}
    };
    
    json mutations = default_initial_point_real(
        instance,
        {"initial_point"},
        json::array(),
        "test"
    );
    
    EXPECT_TRUE(mutations.empty());
}

// ============================================================================
// CheckInterfaceBlock tests
// ============================================================================

TEST(CheckInterfaceBlock, PassesWithAnalysisDrivers) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"simulator.py"}}
        }},
        {"failure_capture", {{"abort", true}}}
    };
    
    json mutations = check_interface_block(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckInterfaceBlock, PassesWithAlgebraicMappings) {
    json instance = {
        {"algebraic_mappings", "mappings.txt"},
        {"failure_capture", {{"abort", true}}}
    };
    
    json mutations = check_interface_block(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckInterfaceBlock, PassesWithBoth) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"simulator.py"}}
        }},
        {"algebraic_mappings", "mappings.txt"},
        {"failure_capture", {{"abort", true}}}
    };
    
    json mutations = check_interface_block(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckInterfaceBlock, ThrowsWhenNeitherPresent) {
    json instance = {
        {"failure_capture", {{"abort", true}}}
    };
    
    EXPECT_THROW(
        check_interface_block(instance, {}, json::array(), "test"),
        ValidationError
    );
}

TEST(CheckInterfaceBlock, ErrorMessageMentionsBothOptions) {
    json instance = {
        {"failure_capture", {{"abort", true}}}
    };
    
    try {
        check_interface_block(instance, {}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("analysis_drivers"), std::string::npos);
        EXPECT_NE(msg.find("algebraic_mappings"), std::string::npos);
    }
}

TEST(CheckInterfaceBlock, BatchRejectsMultipleDrivers) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"driver1.py", "driver2.py"}}
        }},
        {"concurrency", {
            {"batch", json::object()}
        }},
        {"failure_capture", {{"abort", true}}}
    };
    
    EXPECT_THROW(
        check_interface_block(instance, {}, json::array(), "test"),
        ValidationError
    );
}

TEST(CheckInterfaceBlock, BatchRejectsInputFilter) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"simulator.py"}},
            {"input_filter", "preprocess.py"}
        }},
        {"concurrency", {
            {"batch", json::object()}
        }},
        {"failure_capture", {{"abort", true}}}
    };
    
    EXPECT_THROW(
        check_interface_block(instance, {}, json::array(), "test"),
        ValidationError
    );
}

TEST(CheckInterfaceBlock, BatchRejectsOutputFilter) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"simulator.py"}},
            {"output_filter", "postprocess.py"}
        }},
        {"concurrency", {
            {"batch", json::object()}
        }},
        {"failure_capture", {{"abort", true}}}
    };
    
    EXPECT_THROW(
        check_interface_block(instance, {}, json::array(), "test"),
        ValidationError
    );
}

// TEST(CheckInterfaceBlock, BatchRejectsSizeOne) {
//     json instance = {
//         {"analysis_drivers", {
//             {"drivers", {"simulator.py"}}
//         }},
//         {"concurrency", {
//             {"batch", {{"size", 1}}}
//         }},
//         {"failure_capture", {{"abort", true}}}
//     };
    
//     EXPECT_THROW(
//         check_interface_block(instance, {}, json::array(), "test"),
//         ValidationError
//     );
// }

TEST(CheckInterfaceBlock, BatchAllowsLargerSize) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"simulator.py"}}
        }},
        {"concurrency", {
            {"batch", {{"size", 10}}}
        }},
        {"failure_capture", {{"abort", true}}}
    };
    
    json mutations = check_interface_block(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckInterfaceBlock, BatchRejectsDisallowedFailureCapture) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"simulator.py"}}
        }},
        {"concurrency", {
            {"batch", json::object()}
        }},
        {"failure_capture", {
            {"continuation", true}  // Union: only one mode
        }}
    };
    
    try {
        check_interface_block(instance, {}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("failure_capture"), std::string::npos);
        EXPECT_NE(msg.find("continuation"), std::string::npos);
    }
}

TEST(CheckInterfaceBlock, BatchRejectsRetryFailureCapture) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"simulator.py"}}
        }},
        {"concurrency", {
            {"batch", json::object()}
        }},
        {"failure_capture", {
            {"retry", true}
        }}
    };
    
    try {
        check_interface_block(instance, {}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("retry"), std::string::npos);
    }
}

TEST(CheckInterfaceBlock, BatchAllowsAbort) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"simulator.py"}}
        }},
        {"concurrency", {
            {"batch", json::object()}
        }},
        {"failure_capture", {
            {"abort", true}
        }}
    };
    
    json mutations = check_interface_block(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckInterfaceBlock, BatchAllowsRecover) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"simulator.py"}}
        }},
        {"concurrency", {
            {"batch", json::object()}
        }},
        {"failure_capture", {
            {"recover", true}
        }}
    };
    
    json mutations = check_interface_block(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

// TEST(CheckInterfaceBlock, AsyncRejectsBothConcurrencyOne) {
//     json instance = {
//         {"analysis_drivers", {
//             {"drivers", {"simulator.py"}}
//         }},
//         {"concurrency", {
//             {"asynchronous", {
//                 {"evaluation_concurrency", 1},
//                 {"analysis_concurrency", 1}
//             }}
//         }},
//         {"failure_capture", {{"abort", true}}}
//     };
    
//     EXPECT_THROW(
//         check_interface_block(instance, {}, json::array(), "test"),
//         ValidationError
//     );
// }

TEST(CheckInterfaceBlock, AsyncAllowsHigherConcurrency) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"simulator.py"}}
        }},
        {"concurrency", {
            {"asynchronous", {
                {"evaluation_concurrency", 4},
                {"analysis_concurrency", 2}
            }}
        }},
        {"failure_capture", {{"abort", true}}}
    };
    
    json mutations = check_interface_block(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckInterfaceBlock, AsyncAllowsOneConcurrencyOne) {
    json instance = {
        {"analysis_drivers", {
            {"drivers", {"simulator.py"}}
        }},
        {"concurrency", {
            {"asynchronous", {
                {"evaluation_concurrency", 1},
                {"analysis_concurrency", 4}
            }}
        }},
        {"failure_capture", {{"abort", true}}}
    };
    
    json mutations = check_interface_block(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(ValidatorRegistry, HasCheckInterfaceBlock) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_interface_block"));
}

TEST(CheckAnalysisDrivers, PassesWithOneDriver) {
    json instance = {
        {"drivers", {"simulator.py"}}
    };

    json mutations = check_analysis_drivers(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckAnalysisDrivers, RejectsEmptyDrivers) {
    json instance = {
        {"drivers", json::array()}
    };

    EXPECT_THROW(
        check_analysis_drivers(instance, {}, json::array(), "test"),
        ValidationError
    );
}

TEST(CheckAnalysisDrivers, PassesWithEvenAnalysisComponents) {
    json instance = {
        {"drivers", {"d1", "d2"}},
        {"analysis_components", {"c1", "c2", "c3", "c4"}}
    };

    json mutations = check_analysis_drivers(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckAnalysisDrivers, RejectsUnevenAnalysisComponents) {
    json instance = {
        {"drivers", {"d1", "d2"}},
        {"analysis_components", {"c1", "c2", "c3"}}
    };

    EXPECT_THROW(
        check_analysis_drivers(instance, {}, json::array(), "test"),
        ValidationError
    );
}

TEST(ValidatorRegistry, HasCheckAnalysisDrivers) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_analysis_drivers"));
}

// ============================================================================
// CheckResponseDescriptors tests
// ============================================================================

TEST(CheckResponseDescriptors, SkipsWhenDescriptorsProvided) {
    json instance = {
        {"descriptors", {"custom1", "custom2"}},
        {"response_type", {
            {"objective_functions", {{"count", 2}}}
        }}
    };
    
    json mutations = check_response_descriptors(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckResponseDescriptors, GeneratesCalibrationTermsLabels) {
    json instance = {
        {"response_type", {
            {"calibration_terms", {
                {"count", 2},
                {"nonlinear_inequality_constraints", {{"count", 1}}}
            }}
        }}
    };
    
    json mutations = check_response_descriptors(
        instance,
        {},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("descriptors"));
    auto labels = mutations["descriptors"].get<std::vector<std::string>>();
    ASSERT_EQ(labels.size(), 3);
    EXPECT_EQ(labels[0], "least_sq_term_1");
    EXPECT_EQ(labels[1], "least_sq_term_2");
    EXPECT_EQ(labels[2], "nln_ineq_con_1");
}

TEST(CheckResponseDescriptors, GeneratesSingleObjectiveLabel) {
    json instance = {
        {"response_type", {
            {"objective_functions", {{"count", 1}}}
        }}
    };
    
    json mutations = check_response_descriptors(
        instance,
        {},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("descriptors"));
    auto labels = mutations["descriptors"].get<std::vector<std::string>>();
    ASSERT_EQ(labels.size(), 1);
    EXPECT_EQ(labels[0], "obj_fn_1");
}

TEST(CheckResponseDescriptors, GeneratesMultipleObjectiveLabels) {
    json instance = {
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    json mutations = check_response_descriptors(
        instance,
        {},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("descriptors"));
    auto labels = mutations["descriptors"].get<std::vector<std::string>>();
    ASSERT_EQ(labels.size(), 3);
    EXPECT_EQ(labels[0], "obj_fn_1");
    EXPECT_EQ(labels[1], "obj_fn_2");
    EXPECT_EQ(labels[2], "obj_fn_3");
}

TEST(CheckResponseDescriptors, GeneratesResponseFunctionLabels) {
    json instance = {
        {"response_type", {
            {"response_functions", {{"count", 2}}}
        }}
    };
    
    json mutations = check_response_descriptors(
        instance,
        {},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("descriptors"));
    auto labels = mutations["descriptors"].get<std::vector<std::string>>();
    ASSERT_EQ(labels.size(), 2);
    EXPECT_EQ(labels[0], "response_fn_1");
    EXPECT_EQ(labels[1], "response_fn_2");
}

TEST(CheckResponseDescriptors, GeneratesFullOptimizationLabels) {
    json instance = {
        {"response_type", {
            {"objective_functions", {
                {"count", 1},
                {"nonlinear_inequality_constraints", {{"count", 2}}},
                {"nonlinear_equality_constraints", {{"count", 1}}}
            }}
        }}
    };
    
    json mutations = check_response_descriptors(
        instance,
        {},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("descriptors"));
    auto labels = mutations["descriptors"].get<std::vector<std::string>>();
    ASSERT_EQ(labels.size(), 4);
    EXPECT_EQ(labels[0], "obj_fn_1");
    EXPECT_EQ(labels[1], "nln_ineq_con_1");
    EXPECT_EQ(labels[2], "nln_ineq_con_2");
    EXPECT_EQ(labels[3], "nln_eq_con_1");
}

TEST(ValidatorRegistry, HasCheckResponseDescriptors) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_response_descriptors"));
}

// ============================================================================
// CheckFdGradientStepSize tests
// ============================================================================

TEST(CheckFdGradientStepSize, PassesForDakotaSource) {
    json instance = {
        {"gradient_type", {
            {"numerical_gradients", {
                {"method_source", {{"dakota", {}}}},
                {"fd_step_size", {0.001, 0.002, 0.003}}
            }}
        }},
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    json mutations = check_fd_gradient_step_size(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckFdGradientStepSize, PassesForVendorSingleStep) {
    json instance = {
        {"gradient_type", {
            {"numerical_gradients", {
                {"method_source", {{"vendor", true}}},
                {"fd_step_size", {0.001}}
            }}
        }},
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    json mutations = check_fd_gradient_step_size(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckFdGradientStepSize, RaisesForVendorMultipleSteps) {
    json instance = {
        {"gradient_type", {
            {"numerical_gradients", {
                {"method_source", {{"vendor", true}}},
                {"fd_step_size", {0.001, 0.002}}
            }}
        }},
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    EXPECT_THROW(
        check_fd_gradient_step_size(instance, {}, json::array(), "test"),
        ValidationError
    );
}

TEST(ValidatorRegistry, HasCheckFdGradientStepSize) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_fd_gradient_step_size"));
}

// ============================================================================
// CheckMixedGradients tests
// ============================================================================

TEST(CheckMixedGradients, SkipsWhenNotMixed) {
    json instance = {
        {"gradient_type", {
            {"numerical_gradients", {}}
        }},
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    json mutations = check_mixed_gradients(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckMixedGradients, PassesWithValidCoverage) {
    json instance = {
        {"gradient_type", {
            {"mixed_gradients", {
                {"id_analytic_gradients", {1, 3}},
                {"id_numerical_gradients", {2}}
            }}
        }},
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    json mutations = check_mixed_gradients(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckMixedGradients, RaisesOnOutOfRangeId) {
    json instance = {
        {"gradient_type", {
            {"mixed_gradients", {
                {"id_analytic_gradients", {1, 4}},  // 4 is out of range
                {"id_numerical_gradients", {2}}
            }}
        }},
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    EXPECT_THROW(
        check_mixed_gradients(instance, {}, json::array(), "test"),
        ValidationError
    );
}

TEST(CheckMixedGradients, RaisesOnMissingFunction) {
    json instance = {
        {"gradient_type", {
            {"mixed_gradients", {
                {"id_analytic_gradients", {1}},
                {"id_numerical_gradients", {2}}
                // Function 3 is missing
            }}
        }},
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    try {
        check_mixed_gradients(instance, {}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("Function 3"), std::string::npos);
        EXPECT_NE(msg.find("missing"), std::string::npos);
    }
}

TEST(CheckMixedGradients, RaisesOnDuplicateFunction) {
    json instance = {
        {"gradient_type", {
            {"mixed_gradients", {
                {"id_analytic_gradients", {1, 2}},
                {"id_numerical_gradients", {2, 3}}  // 2 is duplicated
            }}
        }},
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    try {
        check_mixed_gradients(instance, {}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("Function 2"), std::string::npos);
        EXPECT_NE(msg.find("replicated"), std::string::npos);
    }
}

TEST(ValidatorRegistry, HasCheckMixedGradients) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_mixed_gradients"));
}

// ============================================================================
// CheckMixedHessians tests
// ============================================================================

TEST(CheckMixedHessians, SkipsWhenNotMixed) {
    json instance = {
        {"hessian_type", {
            {"no_hessians", true}
        }},
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    json mutations = check_mixed_hessians(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckMixedHessians, PassesWithValidCoverage) {
    json instance = {
        {"hessian_type", {
            {"mixed_hessians", {
                {"id_analytic_hessians", {1}},
                {"id_numerical_hessians", {{"values", {2, 3}}}},
                {"id_quasi_hessians", {{"values", {4}}}}
            }}
        }},
        {"response_type", {
            {"objective_functions", {{"count", 4}}}
        }}
    };
    
    json mutations = check_mixed_hessians(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckMixedHessians, RaisesOnMissingFunction) {
    json instance = {
        {"hessian_type", {
            {"mixed_hessians", {
                {"id_analytic_hessians", {1}},
                {"id_numerical_hessians", {{"values", {2}}}}
                // Function 3 is missing
            }}
        }},
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    try {
        check_mixed_hessians(instance, {}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("Function 3"), std::string::npos);
        EXPECT_NE(msg.find("missing"), std::string::npos);
    }
}

TEST(ValidatorRegistry, HasCheckMixedHessians) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_mixed_hessians"));
}

// ============================================================================
// DefaultInequalityBounds tests
// ============================================================================

TEST(DefaultInequalityBounds, SetsDefaultBounds) {
    json instance = {
        {"count", 3}
    };
    
    json mutations = default_inequality_bounds(
        instance,
        {},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("lower_bounds"));
    ASSERT_TRUE(mutations.contains("upper_bounds"));
    
    auto lower = mutations["lower_bounds"].get<std::vector<double>>();
    auto upper = mutations["upper_bounds"].get<std::vector<double>>();
    
    ASSERT_EQ(lower.size(), 3);
    ASSERT_EQ(upper.size(), 3);
    
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(std::isinf(lower[i]) && lower[i] < 0);
        EXPECT_DOUBLE_EQ(upper[i], 0.0);
    }
}

TEST(DefaultInequalityBounds, SkipsWhenAlreadySet) {
    json instance = {
        {"count", 2},
        {"lower_bounds", {-10.0, -20.0}},
        {"upper_bounds", {5.0, 10.0}}
    };
    
    json mutations = default_inequality_bounds(
        instance,
        {},
        json::array(),
        "test"
    );
    
    EXPECT_TRUE(mutations.empty());
}

TEST(DefaultInequalityBounds, SkipsWhenCountZero) {
    json instance = {
        {"count", 0}
    };
    
    json mutations = default_inequality_bounds(
        instance,
        {},
        json::array(),
        "test"
    );
    
    EXPECT_TRUE(mutations.empty());
}

TEST(ValidatorRegistry, HasDefaultInequalityBounds) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("default_inequality_bounds"));
}

// ============================================================================
// DefaultEqualityTargets tests
// ============================================================================

TEST(DefaultEqualityTargets, SetsDefaultTargets) {
    json instance = {
        {"count", 3}
    };
    
    json mutations = default_equality_targets(
        instance,
        {},
        json::array(),
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("targets"));
    
    auto targets = mutations["targets"].get<std::vector<double>>();
    ASSERT_EQ(targets.size(), 3);
    
    for (int i = 0; i < 3; ++i) {
        EXPECT_DOUBLE_EQ(targets[i], 0.0);
    }
}

TEST(DefaultEqualityTargets, SkipsWhenAlreadySet) {
    json instance = {
        {"count", 2},
        {"targets", {1.0, 2.0}}
    };
    
    json mutations = default_equality_targets(
        instance,
        {},
        json::array(),
        "test"
    );
    
    EXPECT_TRUE(mutations.empty());
}

TEST(DefaultEqualityTargets, SkipsWhenCountZero) {
    json instance = {
        {"count", 0}
    };
    
    json mutations = default_equality_targets(
        instance,
        {},
        json::array(),
        "test"
    );
    
    EXPECT_TRUE(mutations.empty());
}

TEST(ValidatorRegistry, HasDefaultEqualityTargets) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("default_equality_targets"));
}

// ============================================================================
// CheckResponseDescriptorsLength tests
// ============================================================================

TEST(CheckResponseDescriptorsLength, SkipsWhenNoDescriptors) {
    json instance = {
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    json mutations = check_response_descriptors_length(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckResponseDescriptorsLength, PassesWhenLengthMatches) {
    json instance = {
        {"descriptors", {"f1", "f2", "f3"}},
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    json mutations = check_response_descriptors_length(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckResponseDescriptorsLength, PassesWithConstraints) {
    json instance = {
        {"descriptors", {"obj", "c1", "c2"}},
        {"response_type", {
            {"objective_functions", {
                {"count", 1},
                {"nonlinear_inequality_constraints", {{"count", 2}}}
            }}
        }}
    };
    
    json mutations = check_response_descriptors_length(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckResponseDescriptorsLength, RaisesWhenLengthMismatch) {
    json instance = {
        {"descriptors", {"f1", "f2"}},  // Only 2, but we have 3
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    try {
        check_response_descriptors_length(instance, {}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("2"), std::string::npos);
        EXPECT_NE(msg.find("3"), std::string::npos);
    }
}

TEST(ValidatorRegistry, HasCheckResponseDescriptorsLength) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_response_descriptors_length"));
}

// ============================================================================
// CheckDescriptorsValid tests (unified validator)
// ============================================================================

TEST(CheckDescriptorsValid, SkipsWhenNoDescriptors) {
    json instance = {
        {"response_type", {
            {"objective_functions", {{"count", 3}}}
        }}
    };
    
    json mutations = check_descriptors_valid(
        instance,
        {"descriptors"},
        {true},  // check_uniqueness
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckDescriptorsValid, PassesValidDescriptors) {
    json instance = {
        {"descriptors", {"obj_fn_1", "constraint_A", "_private"}}
    };
    
    json mutations = check_descriptors_valid(
        instance,
        {"descriptors"},
        {true},
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckDescriptorsValid, PassesSpecialCharacters) {
    // Hyphens, dots, etc. are allowed under relaxed rules
    json instance = {
        {"descriptors", {"obj-fn-1", "constraint.A", "name#2"}}
    };
    
    json mutations = check_descriptors_valid(
        instance,
        {"descriptors"},
        {true},
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckDescriptorsValid, PassesStartsWithDigitButNotNumber) {
    // "2bad" is not a pure number, so it should be allowed
    json instance = {
        {"descriptors", {"2bad", "3rd_place", "4ever"}}
    };
    
    json mutations = check_descriptors_valid(
        instance,
        {"descriptors"},
        {true},
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckDescriptorsValid, RaisesOnDuplicateWhenUniquenessChecked) {
    json instance = {
        {"descriptors", {"f1", "f2", "f1"}}  // f1 is duplicated
    };
    
    try {
        check_descriptors_valid(instance, {"descriptors"}, {true}, "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("Duplicate"), std::string::npos);
        EXPECT_NE(msg.find("f1"), std::string::npos);
    }
}

TEST(CheckDescriptorsValid, AllowsDuplicatesWhenUniquenessNotChecked) {
    json instance = {
        {"descriptors", {"f1", "f1", "f1"}}  // All same - allowed without uniqueness check
    };
    
    json mutations = check_descriptors_valid(
        instance,
        {"descriptors"},
        {false},  // check_uniqueness = false
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckDescriptorsValid, RaisesOnPureNumber) {
    json instance = {
        {"descriptors", {"f1", "123", "f3"}}
    };
    
    try {
        check_descriptors_valid(instance, {"descriptors"}, {true}, "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("123"), std::string::npos);
        EXPECT_NE(msg.find("number"), std::string::npos);
    }
}

TEST(CheckDescriptorsValid, RaisesOnFloatNumber) {
    json instance = {
        {"descriptors", {"f1", "3.14159", "f3"}}
    };
    
    EXPECT_THROW(
        check_descriptors_valid(instance, {"descriptors"}, {true}, "test"),
        ValidationError
    );
}

TEST(CheckDescriptorsValid, RaisesOnScientificNotation) {
    json instance = {
        {"descriptors", {"f1", "1e10", "f3"}}
    };
    
    EXPECT_THROW(
        check_descriptors_valid(instance, {"descriptors"}, {true}, "test"),
        ValidationError
    );
}

TEST(CheckDescriptorsValid, RaisesOnNanInf) {
    for (const std::string& num_str : {"nan", "NaN", "inf", "INF", "-inf", "+infinity"}) {
        json instance = {
            {"descriptors", {"f1", num_str, "f3"}}
        };
        
        EXPECT_THROW(
            check_descriptors_valid(instance, {"descriptors"}, {true}, "test"),
            ValidationError
        ) << "Should reject: " << num_str;
    }
}

TEST(CheckDescriptorsValid, RaisesOnWhitespace) {
    json instance = {
        {"descriptors", {"f1", "bad name", "f3"}}
    };
    
    try {
        check_descriptors_valid(instance, {"descriptors"}, {true}, "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("whitespace"), std::string::npos);
    }
}

TEST(ValidatorRegistry, HasCheckDescriptorsValid) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_descriptors_valid"));
}

// ============================================================================
// CheckPermittedValues tests (generic validator)
// ============================================================================

TEST(CheckPermittedValues, PassesValidTypes) {
    json instance = {
        {"scale_types", {"value", "auto", "log", "none"}}
    };
    
    json literals = {"value", "auto", "log", "none"};
    json mutations = check_permitted_values(
        instance,
        {"scale_types"},
        literals,
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckPermittedValues, RaisesOnInvalidType) {
    json instance = {
        {"scale_types", {"value", "invalid"}}
    };
    
    json literals = {"value", "auto", "log", "none"};
    EXPECT_THROW(
        check_permitted_values(instance, {"scale_types"}, literals, "test"),
        ValidationError
    );
}

TEST(CheckPermittedValues, WorksWithDifferentFieldNames) {
    json instance = {
        {"primary_scale_types", {"value", "log"}}
    };
    
    json literals = {"value", "log", "none"};
    json mutations = check_permitted_values(
        instance,
        {"primary_scale_types"},
        literals,
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(ValidatorRegistry, HasCheckPermittedValues) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_permitted_values"));
}

// ============================================================================
// CheckScalesRequired tests
// ============================================================================

TEST(CheckScalesRequired, PassesWhenNoValueType) {
    json instance = {
        {"scale_types", {"auto", "log"}}
    };
    
    json mutations = check_scales_required(
        instance,
        {"scale_types", "scales"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckScalesRequired, PassesWhenScalesProvided) {
    json instance = {
        {"scale_types", {"value", "auto"}},
        {"scales", {1.0, 2.0}}
    };
    
    json mutations = check_scales_required(
        instance,
        {"scale_types", "scales"},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckScalesRequired, RaisesWhenValueWithoutScales) {
    json instance = {
        {"scale_types", {"value", "auto"}}
    };
    
    EXPECT_THROW(
        check_scales_required(instance, {"scale_types", "scales"}, json::array(), "test"),
        ValidationError
    );
}

TEST(ValidatorRegistry, HasCheckScalesRequired) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_scales_required"));
}

// ============================================================================
// CheckConstraintBoundsOrdering tests
// ============================================================================

TEST(CheckConstraintBoundsOrdering, SkipsWhenNoBounds) {
    json instance = {
        {"count", 3}
    };
    
    json mutations = check_constraint_bounds_ordering(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckConstraintBoundsOrdering, PassesValidOrdering) {
    json instance = {
        {"lower_bounds", {-10.0, -5.0, 0.0}},
        {"upper_bounds", {0.0, 5.0, 10.0}}
    };
    
    json mutations = check_constraint_bounds_ordering(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckConstraintBoundsOrdering, PassesEqualBounds) {
    json instance = {
        {"lower_bounds", {0.0, 5.0}},
        {"upper_bounds", {0.0, 5.0}}
    };
    
    json mutations = check_constraint_bounds_ordering(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckConstraintBoundsOrdering, RaisesWhenLowerExceedsUpper) {
    json instance = {
        {"lower_bounds", {-10.0, 10.0, 0.0}},  // 10.0 > 5.0
        {"upper_bounds", {0.0, 5.0, 10.0}}
    };
    
    try {
        check_constraint_bounds_ordering(instance, {}, json::array(), "test");
        FAIL() << "Expected ValidationError";
    } catch (const ValidationError& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("lower_bounds[1]"), std::string::npos);
        EXPECT_NE(msg.find("exceeds"), std::string::npos);
    }
}

TEST(ValidatorRegistry, HasCheckConstraintBoundsOrdering) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_constraint_bounds_ordering"));
}

// ============================================================================
// Variable validator tests
// ============================================================================

TEST(CheckVariableDescriptorsLength, SkipsWhenNoDescriptors) {
    json instance = {
        {"count", 3}
    };
    
    json mutations = check_variable_descriptors_length(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckVariableDescriptorsLength, PassesWhenLengthMatches) {
    json instance = {
        {"count", 3},
        {"descriptors", {"x1", "x2", "x3"}}
    };
    
    json mutations = check_variable_descriptors_length(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckVariableDescriptorsLength, RaisesWhenLengthMismatch) {
    json instance = {
        {"count", 3},
        {"descriptors", {"x1", "x2"}}  // Only 2
    };
    
    EXPECT_THROW(
        check_variable_descriptors_length(instance, {}, json::array(), "test"),
        ValidationError
    );
}

TEST(ValidatorRegistry, HasCheckVariableDescriptorsLength) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_variable_descriptors_length"));
}

TEST(DefaultVariableDescriptors, SkipsWhenAlreadySet) {
    json instance = {
        {"count", 2},
        {"descriptors", {"custom1", "custom2"}}
    };
    
    json literals = {"cdv_"};
    json mutations = default_variable_descriptors(
        instance,
        {},
        literals,
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(DefaultVariableDescriptors, GeneratesDefaultDescriptors) {
    json instance = {
        {"count", 3}
    };
    
    json literals = {"cdv_"};
    json mutations = default_variable_descriptors(
        instance,
        {},
        literals,
        "test"
    );
    
    ASSERT_TRUE(mutations.contains("descriptors"));
    auto labels = mutations["descriptors"].get<std::vector<std::string>>();
    ASSERT_EQ(labels.size(), 3);
    EXPECT_EQ(labels[0], "cdv_1");
    EXPECT_EQ(labels[1], "cdv_2");
    EXPECT_EQ(labels[2], "cdv_3");
}

TEST(ValidatorRegistry, HasDefaultVariableDescriptors) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("default_variable_descriptors"));
}

TEST(CheckVariableBoundsOrdering, PassesValidOrdering) {
    json instance = {
        {"lower_bounds", {-10.0, 0.0, 1.0}},
        {"upper_bounds", {0.0, 5.0, 10.0}}
    };
    
    json mutations = check_variable_bounds_ordering(
        instance,
        {},
        json::array(),
        "test"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(CheckVariableBoundsOrdering, RaisesWhenLowerExceedsUpper) {
    json instance = {
        {"lower_bounds", {-10.0, 10.0, 1.0}},  // 10.0 > 5.0
        {"upper_bounds", {0.0, 5.0, 10.0}}
    };
    
    EXPECT_THROW(
        check_variable_bounds_ordering(instance, {}, json::array(), "test"),
        ValidationError
    );
}

TEST(ValidatorRegistry, HasCheckVariableBoundsOrdering) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_variable_bounds_ordering"));
}

TEST(ValidatorRegistry, HasCheckLinearInequalityBoundsOrdering) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_linear_inequality_bounds_ordering"));
}

TEST(ValidatorRegistry, HasCheckAllVariableDescriptorsUnique) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("check_all_variable_descriptors_unique"));
}

TEST(UncertainCorrelationMatrixSize, PassesWhenLengthMatchesN2) {
    json instance = {
        {"normal_uncertain", {{"count", 2}}},
        {"uncertain_correlation_matrix", {1.0, 0.0, 0.0, 1.0}}
    };

    json mutations = uncertain_correlation_matrix_size(
        instance,
        {"uncertain_correlation_matrix"},
        json::array(),
        "variables"
    );
    EXPECT_TRUE(mutations.empty());
}

TEST(UncertainCorrelationMatrixSize, RaisesWhenLengthMismatch) {
    json instance = {
        {"normal_uncertain", {{"count", 2}}},
        {"uncertain_correlation_matrix", {1.0, 0.0, 1.0}}
    };

    EXPECT_THROW(
        uncertain_correlation_matrix_size(
            instance,
            {"uncertain_correlation_matrix"},
            json::array(),
            "variables"
        ),
        ValidationError
    );
}

TEST(UncertainCorrelationMatrixSize, RaisesWhenEntryOutsideDomain) {
    json instance = {
        {"normal_uncertain", {{"count", 2}}},
        {"uncertain_correlation_matrix", {1.0, 1.2, 0.0, 1.0}}
    };

    EXPECT_THROW(
        uncertain_correlation_matrix_size(
            instance,
            {"uncertain_correlation_matrix"},
            json::array(),
            "variables"
        ),
        ValidationError
    );
}

TEST(ValidatorRegistry, HasUncertainCorrelationMatrixSize) {
    auto& registry = ValidatorRegistry::instance();
    EXPECT_TRUE(registry.has_validator("uncertain_correlation_matrix_size"));
}

TEST(VariableValidators, NormalUncertainInitialMarksUserProvidedFlag) {
    json instance = {
        {"count", 2},
        {"means", {1.0, 2.0}},
        {"std_deviations", {0.5, 0.5}},
        {"initial_point", {1.5, 2.5}}
    };

    json mutations = normal_uncertain_initial(
        instance,
        {},
        json::array(),
        "normal_uncertain"
    );

    ASSERT_TRUE(mutations.contains("initial_point_user_provided"));
    EXPECT_TRUE(mutations["initial_point_user_provided"].get<bool>());
    EXPECT_FALSE(mutations.contains("initial_point"));
}

TEST(VariableValidators, NormalUncertainInitialLeavesFlagUnsetWhenDefaulted) {
    json instance = {
        {"count", 2},
        {"means", {1.0, 2.0}},
        {"std_deviations", {0.5, 0.5}}
    };

    json mutations = normal_uncertain_initial(
        instance,
        {},
        json::array(),
        "normal_uncertain"
    );

    ASSERT_TRUE(mutations.contains("initial_point"));
    EXPECT_FALSE(mutations.contains("initial_point_user_provided"));
}

// ============================================================================
// Computed field tests
// ============================================================================

TEST(ComputedFieldRegistry, HasHistogramBinLowerBounds) {
    auto& registry = ComputedFieldRegistry::instance();
    EXPECT_TRUE(registry.has_function("histogram_bin_lower_bounds"));
}

TEST(ComputedFieldRegistry, ThrowsOnUnknownFunction) {
    auto& registry = ComputedFieldRegistry::instance();
    EXPECT_THROW(
        registry.compute(json::object(), "not_a_real_computed_field"),
        std::runtime_error
    );
}

TEST(ComputedFields, HistogramBinBoundsUsesExplicitPartitions) {
    json instance = {
        {"count", 2},
        {"pairs_per_variable", {2, 3}},
        {"abscissas", {0.1, 0.4, 1.0, 1.5, 2.0}}
    };

    EXPECT_EQ(histogram_bin_lower_bounds(instance), json::array({0.1, 1.0}));
    EXPECT_EQ(histogram_bin_upper_bounds(instance), json::array({0.4, 2.0}));
}

TEST(ComputedFields, HistogramPointRealBoundsUsesEqualPartitionFallback) {
    json instance = {
        {"count", 2},
        {"abscissas", {3.0, 1.0, 10.0, 7.0}}
    };

    EXPECT_EQ(histogram_point_real_lower_bounds(instance), json::array({1.0, 7.0}));
    EXPECT_EQ(histogram_point_real_upper_bounds(instance), json::array({3.0, 10.0}));
}

TEST(ComputedFields, ContinuousIntervalInferredBoundsRespectPartitions) {
    json instance = {
        {"count", 2},
        {"num_intervals", {2, 3}},
        {"lower_bounds", {-1.0, -3.0, 10.0, 8.0, 9.0}},
        {"upper_bounds", {4.0, 2.0, 11.0, 14.0, 12.0}}
    };

    EXPECT_EQ(continuous_interval_inferred_lower_bounds(instance), json::array({-3.0, 8.0}));
    EXPECT_EQ(continuous_interval_inferred_upper_bounds(instance), json::array({4.0, 14.0}));
}

TEST(ComputedFields, DiscreteSetStringBoundsUseLexicographicMinMax) {
    json instance = {
        {"count", 2},
        {"elements_per_variable", {3, 2}},
        {"elements", {"pear", "apple", "orange", "zulu", "bravo"}}
    };

    EXPECT_EQ(discrete_set_str_lower_bounds(instance), json::array({"apple", "bravo"}));
    EXPECT_EQ(discrete_set_str_upper_bounds(instance), json::array({"pear", "zulu"}));
}

TEST(ComputedFieldRegistry, ComputeDelegatesToRegisteredFunction) {
    auto& registry = ComputedFieldRegistry::instance();
    json instance = {
        {"count", 2},
        {"pairs_per_variable", {2, 3}},
        {"abscissas", {0.1, 0.4, 1.0, 1.5, 2.0}}
    };

    EXPECT_EQ(
        registry.compute(instance, "histogram_bin_upper_bounds"),
        json::array({0.4, 2.0})
    );
}
