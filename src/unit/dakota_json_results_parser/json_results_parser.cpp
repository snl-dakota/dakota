/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "JSONResultsParser.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <cmath>
#include <memory>
#include "dakota_data_types.hpp"
#include <iostream>

using json = nlohmann::json;
using namespace nlohmann::literals;
using namespace Dakota;

using JRP = JSONResultsParser;
using JRPE = JSONResultsParserError;

// Helper to verify that message in a raised exception matches what's expected
auto msg_matches(const std::string& expected) {
    return [expected](const std::exception& ex) {return ex.what() == expected;};
}

// #####################################################################################
// ## Test non-function/gradient/hessian schema
// #####################################################################################

TEST(json_results_parser_tests, test_construct_with_empty_object_throws)
{
  json j;
  EXPECT_THROW(JRP test(j), JRPE); //, msg_matches("JSON results is not an object"));
}

TEST(json_results_parser_tests, test_construct_with_array_throws)
{
  json j = json::array();;
  EXPECT_THROW(JRP test(j), JRPE); //, msg_matches("JSON results is not an object"));
}

TEST(json_results_parser_tests, test_construct_with_fail)
{
  json j;
  j["fail"] = true;
  JRP test(j);
  EXPECT_TRUE(test.failed());
}

TEST(json_results_parser_tests, test_construct_with_not_fail)
{
  json j;
  j["fail"] = false;
  JRP test(j);
  EXPECT_TRUE(!test.failed());
}

// #####################################################################################
// ## Metadata tests
// #####################################################################################

TEST(json_results_parser_tests, test_no_metadata_object_throws)
{
  auto j = json::object();
  JRP test(j);
  EXPECT_THROW(test.metadata("time"), JRPE);
		  //msg_matches("JSON results object does not contain 'metadata', or it's not an object"));
}

TEST(json_results_parser_tests, test_missing_metadata_label_throws)
{
   auto j = R"(
  {
    "metadata": {
        "memory": 1.0
    }
  }
  )"_json;

  JRP test(j);
  EXPECT_THROW(test.metadata("time"), JRPE);
		  //msg_matches("'metadata' object in JSON results object does not contain 'time'"));
}

TEST(json_results_parser_tests, test_metadata_double)
{
  auto j = R"(
  {
    "metadata": {
        "time": 1.0
    }
  }
  )"_json;
 
  JRP test(j);
  auto actual = test.metadata("time");
  EXPECT_TRUE((1.0 == actual));
}

TEST(json_results_parser_tests, test_metadata_non_numeric_throws)
{
  auto j = R"(
  {
    "metadata": {
        "time": "foo"
    }
  }
  )"_json;
      
  JRP test(j);
  EXPECT_THROW(test.metadata("time"), JRPE);
		  //msg_matches("In JSON results object, metadata value for 'time' could not be converted to a number"));
}

TEST(json_results_parser_tests, test_metadata_nonfinite)
{
  auto j = R"(
  {
    "metadata": {
        "time": "nan",
        "year": "inf",
        "month": "-inf"
    }
  }
  )"_json;
      
  JRP test(j);
  EXPECT_TRUE(std::isnan(test.metadata("time")));
  EXPECT_TRUE(std::isinf(test.metadata("year")));
  EXPECT_TRUE(std::isinf(test.metadata("month")));
}


// #####################################################################################
// ## function values tests
// #####################################################################################


TEST(json_results_parser_tests, test_no_functions_object_throws)
{
  auto j = json::object();
  JRP test(j);
  EXPECT_THROW(test.function("response_fn_1"), JRPE);
		  //msg_matches("JSON results object does not contain 'functions', or it's not an object"));
}

TEST(json_results_parser_tests, test_missing_function_label_throws)
{
   auto j = R"(
  {
    "functions": {
        "response_fn_2": 1.0
    }
  }
  )"_json;

  JRP test(j);
  EXPECT_THROW(test.function("response_fn_1"), JRPE);
		  //msg_matches("'functions' object in JSON results object does not contain 'response_fn_1'"));
}

TEST(json_results_parser_tests, test_function_double)
{
  auto j = R"(
  {
    "functions": {
        "response_fn_1": 1.0
    }
  }
  )"_json;
 
  JRP test(j);
  auto actual = test.function("response_fn_1");
  EXPECT_TRUE((1.0 == actual));
}

TEST(json_results_parser_tests, test_function_non_numeric_throws)
{
  auto j = R"(
  {
    "functions": {
        "response_fn_1": "foo"
    }
  }
  )"_json;
      
  JRP test(j);
  EXPECT_THROW(test.function("response_fn_1"), JRPE);
		  //msg_matches("In JSON results object, function value for 'response_fn_1' could not be converted to a number"));
}

TEST(json_results_parser_tests, test_function_nonfinite)
{
  auto j = R"(
  {
    "functions": {
        "response_fn_1": "nan",
        "response_fn_2": "INF",
        "response_fn_3": "-Inf"
    }
  }
  )"_json;
      
  JRP test(j);
  EXPECT_TRUE(std::isnan(test.function("response_fn_1")));
  EXPECT_TRUE(std::isinf(test.function("response_fn_2")));
  EXPECT_TRUE(std::isinf(test.function("response_fn_3")));
}

TEST(json_results_parser_tests, test_function_string_encoded_numbers)
{
  auto j = R"(
  {
    "functions": {
        "response_fn_1": "1",
        "response_fn_2": "2.4",
        "response_fn_3": "3.e9"
    }
  }
  )"_json;
      
  JRP test(j);
  EXPECT_EQ(test.function("response_fn_1"), 1);
  EXPECT_EQ(test.function("response_fn_2"), 2.4);
  EXPECT_EQ(test.function("response_fn_3"), 3.e9);
}


// #####################################################################################
// ## gradents tests
// #####################################################################################


TEST(json_results_parser_tests, test_no_gradients_object_throws) {
  auto j = json::object();
  JRP test(j);
  EXPECT_THROW(test.gradient("response_fn_1"), JRPE);
		  //msg_matches("JSON results object does not contain 'gradients', or it's not an object"));

}

TEST(json_results_parser_tests, test_missing_gradient_label_throws)
{
   auto j = R"(
  {
    "gradients": {
        "response_fn_2": 1.0
    }
  }
  )"_json;

  JRP test(j);
  EXPECT_THROW(test.gradient("response_fn_1"), JRPE);
		  //msg_matches("'gradients' object in JSON results object does not contain 'response_fn_1'"));
}

TEST(json_results_parser_tests, test_gradient_real_vector) {
  RealVector expected(2);
  expected[0] = 1.0, expected[1] = 2.0;
  auto j = R"(
  {
    "gradients": {
        "response_fn_1": [1.0, 2.0]
    }
  }
  )"_json;
  JRP test(j);
  RealVector actual = test.gradient("response_fn_1");
  EXPECT_TRUE((actual == expected));
}

TEST(json_results_parser_tests, test_gradient_not_array_throws)
{
  auto j = R"(
  {
    "gradients": {
        "response_fn_1": {}
    }
  }
  )"_json;
      
  JRP test(j);
  EXPECT_THROW(test.gradient("response_fn_1"), JRPE);
		  //msg_matches("In JSON results object, gradient for 'response_fn_1' is not an array"));
}

TEST(json_results_parser_tests, test_gradient_non_numeric_throws)
{
  auto j = R"(
  {
    "gradients": {
        "response_fn_1": ["foo", "bar"]
    }
  }
  )"_json;
      
  JRP test(j);
  EXPECT_THROW(test.gradient("response_fn_1"), JRPE);
		  //msg_matches("In JSON results object, gradient for 'response_fn_1' contains an element that could not be converted to a number"));
}

TEST(json_results_parser_tests, test_gradient_real_vector_nonfinite) {
  RealVector expected(2);
  auto j = R"(
  {
    "gradients": {
        "response_fn_1": ["NaN", "Inf", "-Inf"]
    }
  }
  )"_json;
  JRP test(j);
  RealVector actual = test.gradient("response_fn_1");
  EXPECT_TRUE(std::isnan(actual[0]));
  EXPECT_TRUE(std::isinf(actual[1]));
  EXPECT_TRUE(std::isinf(actual[2]));
}

// #####################################################################################
// ## hessians tests
// #####################################################################################


TEST(json_results_parser_tests, test_hessian_real_sym_matrix)
{
  RealSymMatrix expected(2);
  expected(1,1) = expected(0,0) = 1.0; expected(1,0) = 2.0;
  auto j = R"(
  {
    "hessians": {
        "response_fn_1": [
                           [1.0, 2.0],
                           [2.0, 1.0]
                         ]
    }
  }
  )"_json;
      
  JRP test(j);
  RealSymMatrix actual = test.hessian("response_fn_1");
  EXPECT_TRUE((actual == expected));
}

TEST(json_results_parser_tests, test_no_hessianss_object_throws) {
  auto j = json::object();
  JRP test(j);
  EXPECT_THROW(test.hessian("response_fn_1"), JRPE);
		  //msg_matches("JSON results object does not contain 'hessians', or it's not an object"));

}

TEST(json_results_parser_tests, test_missing_hessian_label_throws)
{
   auto j = R"(
  {
    "hessians": {
        "response_fn_2": []
    }
  }
  )"_json;

  JRP test(j);
  EXPECT_THROW(test.hessian("response_fn_1"), JRPE);
		  //msg_matches("'hessians' object in JSON results object does not contain 'response_fn_1'"));
}

TEST(json_results_parser_tests, test_hessians_not_array_throws)
{
  auto j = R"(
  {
    "hessians": {
        "response_fn_1": {}
    }
  }
  )"_json;
      
  JRP test(j);
  EXPECT_THROW(test.hessian("response_fn_1"), JRPE);
		  //msg_matches("In JSON results object, Hessian for 'response_fn_1' is not an array"));
}

TEST(json_results_parser_tests, test_hessian_row_not_array_throws)
{
  auto j = R"(
  {
    "hessians": {
        "response_fn_1": [ [1.0, 2.0], {} ]
    }
  }
  )"_json;
      
  JRP test(j);
  EXPECT_THROW(test.hessian("response_fn_1"), JRPE);
		  //msg_matches("In JSON results object, Hessian for 'response_fn_1' contains a row that is not an array"));
}

TEST(json_results_parser_tests, test_hessian_non_square_throws)
{
  auto j = R"(
  {
    "hessians": {
        "response_fn_1": [ [1.0, 2.0], [1.0, 20, 3.0] ]
    }
  }
  )"_json;
      
  JRP test(j);
  EXPECT_THROW(test.hessian("response_fn_1"), JRPE);
		  //msg_matches("In JSON results object, Hessian for 'response_fn_1' is not square"));
}

TEST(json_results_parser_tests, test_hessian_non_numeric_throws)
{
  auto j = R"(
  {
    "hessians": {
        "response_fn_1": [[1.0, 2.0], ["foo", "bar"]]
    }
  }
  )"_json;
      
  JRP test(j);
  EXPECT_THROW(test.hessian("response_fn_1"), JRPE);
		  //msg_matches("In JSON results object, Hessian for 'response_fn_1' contains an element that could not be converted to a number"));
}

TEST(json_results_parser_tests, test_hessian_real_sym_matrix_nonfinite)
{
  auto j = R"(
  {
    "hessians": {
        "response_fn_1": [
                           ["NAN", 2.0],
                           [2.0, "inF"]
                         ]
    }
  }
  )"_json;
      
  JRP test(j);
  RealSymMatrix actual = test.hessian("response_fn_1");
  EXPECT_TRUE(std::isnan(actual(0,0)));
  EXPECT_TRUE(std::isinf(actual(1,1)));
  EXPECT_EQ(actual(1,0), 2.0);
  EXPECT_EQ(actual(0,1), 2.0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
