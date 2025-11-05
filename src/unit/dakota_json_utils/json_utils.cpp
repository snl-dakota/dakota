/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "JSONUtils.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <cmath>
#include <memory>
#include "dakota_data_types.hpp"
#include <iostream>

using json = nlohmann::json;
using namespace nlohmann::literals;
using namespace Dakota;


//TEST(json_utils_tests, test_real_matrix_array) {
//  EXPECT_TRUE((actual == expected));
//}

// -----------------------------------

TEST(json_utils_tests, test_real_vector) {
  RealVector expected(3);
  expected[0] = 1.0, expected[1] = 2.0, expected[2] = 3.14;
  auto j = R"(
  {
      "test_values": [1.0, 2.0, 3.14]
  }
  )"_json;

  auto actual = j["test_values"].template get<JSONRealVector>().value;
  EXPECT_TRUE((actual == expected));
}

// -----------------------------------

TEST(json_utils_tests, test_int_vector) {
  IntVector expected(2);
  expected[0] = -2, expected[1] = 3;
  auto j = R"(
  {
      "test_values": [-2, 3]
  }
  )"_json;

  auto actual = j["test_values"].template get<JSONIntVector>().value;
  EXPECT_TRUE((actual == expected));
}

// -----------------------------------

TEST(json_utils_tests, test_real_sym_matrix)
{
  RealSymMatrix expected(2);
  expected(1,1) = expected(0,0) = 1.0; expected(1,0) = 2.0;
  auto j = R"(
  {
      "test_values": [
                         [1.0, 2.0],
                         [2.0, 1.0]
                       ]
  }
  )"_json;
      
  auto actual = j["test_values"].template get<JSONRealSymMatrix>().value;
  EXPECT_TRUE((actual == expected));
}

//TEST(json_utils_tests, test_real_vector_array) {
//  EXPECT_TRUE((actual == expected));
//}

// -----------------------------------

//TEST(json_utils_tests, test_int_vector_array) {
//  EXPECT_TRUE((actual == expected));
//}

// -----------------------------------

TEST(json_utils_tests, test_int_set) {
  IntSet expected = { 2, -1, 0, 4, 2 };
  auto j = R"(
  {
      "test_values": [0, -1, 4, 2]
  }
  )"_json;

  // This uses nlohmann native support for std::set<>
  // ... and entries respect the container properties
  auto actual = j["test_values"].template get<IntSet>(); // no .value access needed

  EXPECT_TRUE((actual == expected));
}

// -----------------------------------

TEST(json_utils_tests, test_int_set_array) {
  IntSetArray expected = { { 2, -1, 0, 4, 2 },
                           { -3, 3 } };
  auto j = R"(
  {
      "test_values": [ [0, -1, 4, 2],
                       [3, -3]       ]
  }
  )"_json;

  // This uses nlohmann native support for std::vector<std::set<>>
  // ... and entries respect the container properties
  auto actual = j["test_values"].template get<IntSetArray>(); // no .value access needed

  EXPECT_TRUE((actual == expected));
}

// -----------------------------------

TEST(json_utils_tests, test_string_set_array) {
  StringSetArray expected = { { "c1", "b2", "b1", "a" },
                           { "c", "e", "c" } };
  auto j = R"(
  {
      "test_values": [ ["a", "c1", "b1", "b2"],
                       ["e", "c"]              ]
  }
  )"_json;

  // This uses nlohmann native support for std::vector<std::set<>>
  // ... and entries respect the container properties
  auto actual = j["test_values"].template get<StringSetArray>(); // no .value access needed

  EXPECT_TRUE((actual == expected));
}

// -----------------------------------

TEST(json_utils_tests, test_int_real_map) {
  IntRealMap expected = { {-1, 1.23} ,
                          { 2, 3.21} ,
                          { 0, 1.0 }  };
  auto j = R"(
  {
      "test_values": [ [ 0, 1.0 ],
                       [-1, 1.23],
                       [ 2, 3.21] ]
  }
  )"_json;

  // This uses nlohmann native support for std::map<>
  auto actual = j["test_values"].template get<IntRealMap>(); // no .value access needed

  EXPECT_TRUE((actual == expected));
}

// -----------------------------------

TEST(json_utils_tests, test_int_real_map_array) {
  IntRealMapArray expected = {
                               { {-1, 1.23} , { 2, 3.21} , { 0, 1.0 } },
                               { {-3, -1.2} , { 3, 1.2 } }
                             };
  auto j = R"(
  {
      "test_values": [ [ [ 0,  1.0 ], [-1, 1.23], [2, 3.21] ],
                       [ [-3, -1.2 ], [ 3, 1.2 ] ] ]
  }
  )"_json;

  // This uses nlohmann native support for std::vector<std::map<>>
  auto actual = j["test_values"].template get<IntRealMapArray>(); // no .value access needed

  EXPECT_TRUE((actual == expected));
}

// -----------------------------------

TEST(json_utils_tests, test_int_int_pair_real_map_array) {
  IntIntPairRealMapArray
    expected = { {
                   { {0, 1}, 1.23 },
                   { {2, 0}, 3.21 }
                 },
                 {
                   { {4, 1}, 0.23 },
                   { {4, 2}, 1.21 },
                   { {4, 3}, 2.21 }
                 }
               };

  auto j = R"(
  {
      "test_values": [ [
                         [ [0, 1], 1.23 ],
                         [ [2, 0], 3.21 ]
                       ],
                       [
                         [ [4, 1], 0.23 ],
                         [ [4, 2], 1.21 ],
                         [ [4, 3], 2.21 ]
                       ]
                     ]
  }
  )"_json;

  // This uses nlohmann native support for std::vector<std::map<std::pair<>,Real>>
  auto actual = j["test_values"].template get<IntIntPairRealMapArray>(); // no .value access needed

  EXPECT_TRUE((actual == expected));
}

// -----------------------------------

TEST(json_utils_tests, test_string_array) {
  StringArray expected = { "val1", "val2" };
  auto j = R"(
  {
      "test_values": ["val1", "val2"]
  }
  )"_json;

  // This uses nlohmann native support for std::vector<>
  auto actual = j["test_values"].template get<StringArray>(); // no .value access needed

  EXPECT_TRUE((actual == expected));
}

// -----------------------------------

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
