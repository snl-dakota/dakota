// test_schema_validator.cpp
// Unit tests for Dakota Schema Validator

#include <gtest/gtest.h>
#include <dakota/schema_validator.hpp>
#include <dakota/validation.hpp>
#include <dakota/computed_fields.hpp>

using namespace dakota::schema;
using json = nlohmann::json;

// ============================================================================
// Utility Function Tests
// ============================================================================

TEST(UtilityTests, ParseRefName) {
    EXPECT_EQ(parse_ref_name("#/$defs/MyModel"), "MyModel");
    EXPECT_EQ(parse_ref_name("#/$defs/ContinuousDesign"), "ContinuousDesign");
    EXPECT_EQ(parse_ref_name("OtherRef"), "OtherRef");
}

TEST(UtilityTests, FormatPath) {
    EXPECT_EQ(format_path({}), "");
    EXPECT_EQ(format_path({"method"}), "method");
    EXPECT_EQ(format_path({"method", "[0]"}), "method[0]");
    EXPECT_EQ(format_path({"method", "[0]", "sampling"}), "method[0].sampling");
    EXPECT_EQ(format_path({"method", "[0]", "sampling", "count"}), "method[0].sampling.count");
}

TEST(UtilityTests, MatchesType) {
    EXPECT_TRUE(matches_type(json(42), SchemaNode::Type::Integer));
    EXPECT_TRUE(matches_type(json(3.14), SchemaNode::Type::Number));
    EXPECT_TRUE(matches_type(json("hello"), SchemaNode::Type::String));
    EXPECT_TRUE(matches_type(json(true), SchemaNode::Type::Boolean));
    EXPECT_TRUE(matches_type(json(nullptr), SchemaNode::Type::Null));
    EXPECT_TRUE(matches_type(json::array(), SchemaNode::Type::Array));
    EXPECT_TRUE(matches_type(json::object(), SchemaNode::Type::Object));
    
    // Integer also matches Number
    EXPECT_TRUE(matches_type(json(42), SchemaNode::Type::Number));
    
    // Negative tests
    EXPECT_FALSE(matches_type(json("42"), SchemaNode::Type::Integer));
    EXPECT_FALSE(matches_type(json(42), SchemaNode::Type::String));
}

TEST(UtilityTests, TypeName) {
    EXPECT_EQ(type_name(SchemaNode::Type::Object), "object");
    EXPECT_EQ(type_name(SchemaNode::Type::Array), "array");
    EXPECT_EQ(type_name(SchemaNode::Type::String), "string");
    EXPECT_EQ(type_name(SchemaNode::Type::Integer), "integer");
    EXPECT_EQ(type_name(SchemaNode::Type::Number), "number");
}

TEST(UtilityTests, JsonTypeName) {
    EXPECT_EQ(json_type_name(json(42)), "integer");
    EXPECT_EQ(json_type_name(json(3.14)), "number");
    EXPECT_EQ(json_type_name(json("hello")), "string");
    EXPECT_EQ(json_type_name(json(true)), "boolean");
    EXPECT_EQ(json_type_name(json(nullptr)), "null");
}

// ============================================================================
// SchemaValidationResult Tests
// ============================================================================

TEST(ValidationResultTests, EmptyIsValid) {
    SchemaValidationResult result;
    EXPECT_TRUE(result.is_valid());
    EXPECT_TRUE(result.errors().empty());
}

TEST(ValidationResultTests, AddError) {
    SchemaValidationResult result;
    result.add_error({"method[0].count", "Expected integer, got string", "type"});
    
    EXPECT_FALSE(result.is_valid());
    EXPECT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0].path, "method[0].count");
    EXPECT_EQ(result.errors()[0].message, "Expected integer, got string");
    EXPECT_EQ(result.errors()[0].rule, "type");
}

TEST(ValidationResultTests, Format) {
    SchemaValidationResult result;
    result.add_error({"field1", "Error 1", "rule1"});
    result.add_error({"field2", "Error 2", "rule2"});
    
    std::string formatted = result.format();
    EXPECT_TRUE(formatted.find("2 validation errors") != std::string::npos);
    EXPECT_TRUE(formatted.find("field1") != std::string::npos);
    EXPECT_TRUE(formatted.find("field2") != std::string::npos);
}

TEST(ValidationResultTests, RaiseIfInvalid) {
    SchemaValidationResult valid_result;
    EXPECT_NO_THROW(valid_result.raise_if_invalid());
    
    SchemaValidationResult invalid_result;
    invalid_result.add_error({"field", "error", "rule"});
    EXPECT_THROW(invalid_result.raise_if_invalid(), SchemaValidationException);
}

// ============================================================================
// Schema Loading Tests
// ============================================================================

TEST(SchemaLoadingTests, SimpleSchema) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"value", {{"type", "integer"}}}
        }},
        {"required", {"name"}}
    };
    
    SchemaValidator validator(schema);
    EXPECT_EQ(validator.root().type, SchemaNode::Type::Object);
    EXPECT_EQ(validator.root().properties.size(), 2);
    EXPECT_EQ(validator.root().required.size(), 1);
}

TEST(SchemaLoadingTests, SchemaWithDefs) {
    json schema = {
        {"$defs", {
            {"SimpleModel", {
                {"type", "object"},
                {"properties", {
                    {"value", {{"type", "integer"}}}
                }}
            }}
        }},
        {"type", "object"},
        {"properties", {
            {"item", {{"$ref", "#/$defs/SimpleModel"}}}
        }}
    };
    
    SchemaValidator validator(schema);
    auto def = validator.get_definition("SimpleModel");
    EXPECT_NE(def, nullptr);
    EXPECT_EQ(def->type, SchemaNode::Type::Object);
}

// ============================================================================
// Basic Validation Tests
// ============================================================================

TEST(ValidationTests, ValidObject) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"value", {{"type", "integer"}}}
        }},
        {"required", {"name", "value"}},
        {"additionalProperties", false}
    };
    
    SchemaValidator validator(schema);
    
    json valid = {{"name", "test"}, {"value", 42}};
    auto [result, validation] = validator.validate_safe(valid);
    EXPECT_TRUE(validation.is_valid());
}

TEST(ValidationTests, MissingRequired) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"value", {{"type", "integer"}}}
        }},
        {"required", {"name", "value"}}
    };
    
    SchemaValidator validator(schema);
    
    json missing = {{"name", "test"}};  // Missing "value"
    auto [result, validation] = validator.validate_safe(missing);
    EXPECT_FALSE(validation.is_valid());
    EXPECT_EQ(validation.errors().size(), 1);
    EXPECT_TRUE(validation.errors()[0].message.find("value") != std::string::npos);
}

TEST(ValidationTests, WrongType) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"value", {{"type", "integer"}}}
        }}
    };
    
    SchemaValidator validator(schema);
    
    json wrong = {{"value", "not an integer"}};
    auto [result, validation] = validator.validate_safe(wrong);
    EXPECT_FALSE(validation.is_valid());
}

TEST(ValidationTests, AdditionalProperties) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}}
        }},
        {"additionalProperties", false}
    };
    
    SchemaValidator validator(schema);
    
    json extra = {{"name", "test"}, {"extra", "not allowed"}};
    auto [result, validation] = validator.validate_safe(extra);
    EXPECT_FALSE(validation.is_valid());
    EXPECT_TRUE(validation.errors()[0].message.find("extra") != std::string::npos);
}

// ============================================================================
// Numeric Constraint Tests
// ============================================================================

TEST(ConstraintTests, GreaterThan) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"count", {{"type", "integer"}, {"gt", 0}}}
        }}
    };
    
    SchemaValidator validator(schema);
    
    auto [r1, v1] = validator.validate_safe({{"count", 1}});
    EXPECT_TRUE(v1.is_valid());
    
    auto [r2, v2] = validator.validate_safe({{"count", 0}});
    EXPECT_FALSE(v2.is_valid());
    
    auto [r3, v3] = validator.validate_safe({{"count", -1}});
    EXPECT_FALSE(v3.is_valid());
}

TEST(ConstraintTests, Minimum) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"count", {{"type", "integer"}, {"minimum", 0}}}
        }}
    };
    
    SchemaValidator validator(schema);
    
    auto [r1, v1] = validator.validate_safe({{"count", 0}});
    EXPECT_TRUE(v1.is_valid());
    
    auto [r2, v2] = validator.validate_safe({{"count", -1}});
    EXPECT_FALSE(v2.is_valid());
}

// ============================================================================
// Array Validation Tests
// ============================================================================

TEST(ArrayTests, ValidArray) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"items", {
                {"type", "array"},
                {"items", {{"type", "integer"}}}
            }}
        }}
    };
    
    SchemaValidator validator(schema);
    
    auto [r1, v1] = validator.validate_safe({{"items", {1, 2, 3}}});
    EXPECT_TRUE(v1.is_valid());
}

TEST(ArrayTests, InvalidArrayItems) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"items", {
                {"type", "array"},
                {"items", {{"type", "integer"}}}
            }}
        }}
    };
    
    SchemaValidator validator(schema);
    
    auto [r1, v1] = validator.validate_safe({{"items", {1, "two", 3}}});
    EXPECT_FALSE(v1.is_valid());
}

// ============================================================================
// Union Pattern Tests
// ============================================================================

TEST(UnionTests, Pattern2Null) {
    json schema = {
        {"$defs", {
            {"OptionA", {
                {"type", "object"},
                {"properties", {
                    {"option_a", {{"const", true}, {"default", true}}}
                }},
                {"required", {"option_a"}}
            }}
        }},
        {"type", "object"},
        {"properties", {
            {"selection", {
                {"anyOf", {
                    {{"$ref", "#/$defs/OptionA"}},
                    {{"type", "null"}}
                }},
                {"x-union-pattern", 2},
                {"default", nullptr}
            }}
        }}
    };
    
    SchemaValidator validator(schema);
    
    // Null should be valid for Pattern 2
    auto [r1, v1] = validator.validate_safe({{"selection", nullptr}});
    EXPECT_TRUE(v1.is_valid());
}

TEST(UnionTests, Pattern4Required) {
    json schema = {
        {"$defs", {
            {"OptionA", {
                {"type", "object"},
                {"properties", {
                    {"option_a", {{"const", true}}}
                }},
                {"required", {"option_a"}}
            }},
            {"OptionB", {
                {"type", "object"},
                {"properties", {
                    {"option_b", {{"const", true}}}
                }},
                {"required", {"option_b"}}
            }}
        }},
        {"type", "object"},
        {"properties", {
            {"selection", {
                {"anyOf", {
                    {{"$ref", "#/$defs/OptionA"}},
                    {{"$ref", "#/$defs/OptionB"}}
                }},
                {"x-union-pattern", 4}
            }}
        }},
        {"required", {"selection"}}
    };
    
    SchemaValidator validator(schema);
    
    // Valid discriminator
    auto [r1, v1] = validator.validate_safe({{"selection", {{"option_a", true}}}});
    EXPECT_TRUE(v1.is_valid());
    
    // Invalid discriminator
    auto [r2, v2] = validator.validate_safe({{"selection", {{"invalid", true}}}});
    EXPECT_FALSE(v2.is_valid());
    
    // Multiple keys not allowed
    auto [r3, v3] = validator.validate_safe({{"selection", {{"option_a", true}, {"option_b", true}}}});
    EXPECT_FALSE(v3.is_valid());
}

// ============================================================================
// Default Value Tests
// ============================================================================

TEST(DefaultTests, ApplyDefaults) {
    json schema = {
        {"type", "object"},
        {"properties", {
            {"name", {{"type", "string"}}},
            {"count", {{"type", "integer"}, {"default", 10}}}
        }}
    };
    
    SchemaValidator validator(schema);
    
    json input = {{"name", "test"}};
    auto [result, validation] = validator.validate_safe(input);
    EXPECT_TRUE(validation.is_valid());
    EXPECT_EQ(result["count"], 10);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
