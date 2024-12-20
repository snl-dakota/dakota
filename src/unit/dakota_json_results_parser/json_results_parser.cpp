/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "JSONResultsParser.hpp"

#define BOOST_TEST_MODULE dakota_json_results_parser
#include <boost/test/included/unit_test.hpp>
#include <boost/test/tools/detail/per_element_manip.hpp>
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

BOOST_AUTO_TEST_CASE(test_construct_with_empty_object_throws)
{
  json j;
  BOOST_CHECK_EXCEPTION(JRP test(j), JRPE, msg_matches("JSON results is not an object"));
}

BOOST_AUTO_TEST_CASE(test_construct_with_array_throws)
{
  json j = json::array();;
  BOOST_CHECK_EXCEPTION(JRP test(j), JRPE, msg_matches("JSON results is not an object"));
}

BOOST_AUTO_TEST_CASE(test_construct_with_fail)
{
  json j;
  j["fail"] = true;
  JRP test(j);
  BOOST_CHECK(test.failed());
}

BOOST_AUTO_TEST_CASE(test_construct_with_not_fail)
{
  json j;
  j["fail"] = false;
  JRP test(j);
  BOOST_CHECK(!test.failed());
}

// #####################################################################################
// ## Metadata tests
// #####################################################################################

BOOST_AUTO_TEST_CASE(test_no_metadata_object_throws)
{
  auto j = json::object();
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.metadata("time"), JRPE,
		  msg_matches("JSON results object does not contain 'metadata', or it's not an object"));
}

BOOST_AUTO_TEST_CASE(test_missing_metadata_label_throws)
{
   auto j = R"(
  {
    "metadata": {
        "memory": 1.0
    }
  }
  )"_json;

  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.metadata("time"), JRPE,
		  msg_matches("'metadata' object in JSON results object does not contain 'time'"));
}

BOOST_AUTO_TEST_CASE(test_metadata_double)
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
  BOOST_CHECK(1.0 == actual);
}

BOOST_AUTO_TEST_CASE(test_metadata_non_numeric_throws)
{
  auto j = R"(
  {
    "metadata": {
        "time": "foo"
    }
  }
  )"_json;
      
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.metadata("time"), JRPE,
		  msg_matches("In JSON results object, metadata value for 'time' could not be converted to a number"));
}

BOOST_AUTO_TEST_CASE(test_metadata_nonfinite)
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
  BOOST_CHECK(std::isnan(test.metadata("time")));
  BOOST_CHECK(std::isinf(test.metadata("year")));
  BOOST_CHECK(std::isinf(test.metadata("month")));
}


// #####################################################################################
// ## function values tests
// #####################################################################################


BOOST_AUTO_TEST_CASE(test_no_functions_object_throws)
{
  auto j = json::object();
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.function("response_fn_1"), JRPE,
		  msg_matches("JSON results object does not contain 'functions', or it's not an object"));
}

BOOST_AUTO_TEST_CASE(test_missing_function_label_throws)
{
   auto j = R"(
  {
    "functions": {
        "response_fn_2": 1.0
    }
  }
  )"_json;

  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.function("response_fn_1"), JRPE,
		  msg_matches("'functions' object in JSON results object does not contain 'response_fn_1'"));
}

BOOST_AUTO_TEST_CASE(test_function_double)
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
  BOOST_CHECK(1.0 == actual);
}

BOOST_AUTO_TEST_CASE(test_function_non_numeric_throws)
{
  auto j = R"(
  {
    "functions": {
        "response_fn_1": "foo"
    }
  }
  )"_json;
      
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.function("response_fn_1"), JRPE,
		  msg_matches("In JSON results object, function value for 'response_fn_1' could not be converted to a number"));
}

BOOST_AUTO_TEST_CASE(test_function_nonfinite)
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
  BOOST_CHECK(std::isnan(test.function("response_fn_1")));
  BOOST_CHECK(std::isinf(test.function("response_fn_2")));
  BOOST_CHECK(std::isinf(test.function("response_fn_3")));
}

BOOST_AUTO_TEST_CASE(test_function_string_encoded_numbers)
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
  BOOST_CHECK_EQUAL(test.function("response_fn_1"), 1);
  BOOST_CHECK_EQUAL(test.function("response_fn_2"), 2.4);
  BOOST_CHECK_EQUAL(test.function("response_fn_3"), 3.e9);
}


// #####################################################################################
// ## gradents tests
// #####################################################################################


BOOST_AUTO_TEST_CASE(test_no_gradients_object_throws) {
  auto j = json::object();
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.gradient("response_fn_1"), JRPE,
		  msg_matches("JSON results object does not contain 'gradients', or it's not an object"));

}

BOOST_AUTO_TEST_CASE(test_missing_gradient_label_throws)
{
   auto j = R"(
  {
    "gradients": {
        "response_fn_2": 1.0
    }
  }
  )"_json;

  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.gradient("response_fn_1"), JRPE,
		  msg_matches("'gradients' object in JSON results object does not contain 'response_fn_1'"));
}

BOOST_AUTO_TEST_CASE(test_gradient_real_vector) {
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
  BOOST_CHECK(actual == expected);
}

BOOST_AUTO_TEST_CASE(test_gradient_not_array_throws)
{
  auto j = R"(
  {
    "gradients": {
        "response_fn_1": {}
    }
  }
  )"_json;
      
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.gradient("response_fn_1"), JRPE,
		  msg_matches("In JSON results object, gradient for 'response_fn_1' is not an array"));
}

BOOST_AUTO_TEST_CASE(test_gradient_non_numeric_throws)
{
  auto j = R"(
  {
    "gradients": {
        "response_fn_1": ["foo", "bar"]
    }
  }
  )"_json;
      
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.gradient("response_fn_1"), JRPE,
		  msg_matches("In JSON results object, gradient for 'response_fn_1' contains an element that could not be converted to a number"));
}

BOOST_AUTO_TEST_CASE(test_gradient_real_vector_nonfinite) {
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
  BOOST_CHECK(std::isnan(actual[0]));
  BOOST_CHECK(std::isinf(actual[1]));
  BOOST_CHECK(std::isinf(actual[2]));
}

// #####################################################################################
// ## hessians tests
// #####################################################################################


BOOST_AUTO_TEST_CASE(test_hessian_real_sym_matrix)
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
  BOOST_CHECK(actual == expected);
}

BOOST_AUTO_TEST_CASE(test_no_hessianss_object_throws) {
  auto j = json::object();
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.hessian("response_fn_1"), JRPE,
		  msg_matches("JSON results object does not contain 'hessians', or it's not an object"));

}

BOOST_AUTO_TEST_CASE(test_missing_hessian_label_throws)
{
   auto j = R"(
  {
    "hessians": {
        "response_fn_2": []
    }
  }
  )"_json;

  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.hessian("response_fn_1"), JRPE,
		  msg_matches("'hessians' object in JSON results object does not contain 'response_fn_1'"));
}

BOOST_AUTO_TEST_CASE(test_hessians_not_array_throws)
{
  auto j = R"(
  {
    "hessians": {
        "response_fn_1": {}
    }
  }
  )"_json;
      
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.hessian("response_fn_1"), JRPE,
		  msg_matches("In JSON results object, Hessian for 'response_fn_1' is not an array"));
}

BOOST_AUTO_TEST_CASE(test_hessian_row_not_array_throws)
{
  auto j = R"(
  {
    "hessians": {
        "response_fn_1": [ [1.0, 2.0], {} ]
    }
  }
  )"_json;
      
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.hessian("response_fn_1"), JRPE,
		  msg_matches("In JSON results object, Hessian for 'response_fn_1' contains a row that is not an array"));
}

BOOST_AUTO_TEST_CASE(test_hessian_non_square_throws)
{
  auto j = R"(
  {
    "hessians": {
        "response_fn_1": [ [1.0, 2.0], [1.0, 20, 3.0] ]
    }
  }
  )"_json;
      
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.hessian("response_fn_1"), JRPE,
		  msg_matches("In JSON results object, Hessian for 'response_fn_1' is not square"));
}

BOOST_AUTO_TEST_CASE(test_hessian_non_numeric_throws)
{
  auto j = R"(
  {
    "hessians": {
        "response_fn_1": [[1.0, 2.0], ["foo", "bar"]]
    }
  }
  )"_json;
      
  JRP test(j);
  BOOST_CHECK_EXCEPTION(test.hessian("response_fn_1"), JRPE,
		  msg_matches("In JSON results object, Hessian for 'response_fn_1' contains an element that could not be converted to a number"));
}

BOOST_AUTO_TEST_CASE(test_hessian_real_sym_matrix_nonfinite)
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
  BOOST_CHECK(std::isnan(actual(0,0)));
  BOOST_CHECK(std::isinf(actual(1,1)));
  BOOST_CHECK_EQUAL(actual(1,0), 2.0);
  BOOST_CHECK_EQUAL(actual(0,1), 2.0);
}


