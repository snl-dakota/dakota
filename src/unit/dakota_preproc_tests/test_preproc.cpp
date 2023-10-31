/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// TODO: How to manage these tests, given that they require external
// tools and PATH settings? Maybe configure_file(pyprepro.py) and set env
// Maybe CMake Test property for ENVIRONMENT

#include "dakota_preproc_util.hpp"
#include <boost/filesystem.hpp>
#define BOOST_TEST_MODULE dakota_preproc
#include <boost/test/included/unit_test.hpp>


BOOST_AUTO_TEST_CASE(test_pyprepro)
{
  // generate an input and diff against the baseline
  std::string tmpl_file("preproc_dakota.tmpl");
  std::string gen_file = Dakota::pyprepro_input(tmpl_file);

  // TODO: check_equal_files() function with this test:
  std::ifstream base_ifs("preproc_dakota.base");
  std::ifstream test_ifs(gen_file);
  std::istream_iterator<char> base_it(base_ifs), base_end;
  std::istream_iterator<char> test_it(test_ifs), test_end;
  BOOST_CHECK_EQUAL_COLLECTIONS(base_it, base_end, test_it, test_end);

  // remove generated file
  boost::filesystem::remove(gen_file);
}
