/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// TODO: How to manage these tests, given that they require external
// tools and PATH settings? Maybe configure_file(pyprepro.py) and set env
// Maybe CMake Test property for ENVIRONMENT

#include "dakota_preproc_util.hpp"
#include <boost/filesystem/operations.hpp>
#include <gtest/gtest.h>

#include <fstream>

TEST(test_preproc_tests, test_pyprepro)
{
  // generate an input and diff against the baseline
  std::string tmpl_file("preproc_dakota.tmpl");
  std::string gen_file = Dakota::pyprepro_input(tmpl_file);

  // TODO: check_equal_files() function with this test:
  std::ifstream base_ifs("preproc_dakota.base");
  std::ifstream test_ifs(gen_file);
  std::istream_iterator<char> base_it(base_ifs), base_end;
  std::istream_iterator<char> test_it(test_ifs), test_end;
  //BOOST_CHECK_EQUAL_COLLECTIONS(base_it, base_end, test_it, test_end);
  for (; (base_it != base_end) && (test_it != test_end); ++base_it, ++test_it) {
    EXPECT_EQ(*base_it, *test_it);
  }

  // remove generated file
  boost::filesystem::remove(gen_file);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
