/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


/** \file redirect_regexs.cpp Verify regexs used to parse input file
    redirection options. */

#include "OutputManager.hpp"

#define BOOST_TEST_MODULE dakota_redirect_regexs
#include <boost/test/included/unit_test.hpp>

// These examples should cause redirects
std::string valid_redirs = R"(
environment
output_file  =    'dakota.log'
some_other_keyword
error_file "dakota.err"
)";

std::string valid_redirs_comments = R"(
environment
output_file  =    'dakota.log'# trailing comment
some_other_keyword
# full line comment
error_file "dakota.err" # trailing comment
)";

std::string valid_redirs_intervening_lines = R"(
environment
output_file  # where to output
  # If you really want to,
  # change the log here:
  'dakota.log'# trailing comment
some_other_keyword
# full line comment
error_file 
  =          # so it equals
  # change the error file here:
  "dakota.err" # trailing comment
)";

// These commented lines should not cause a redirect
std::string no_redirs_comments = R"(
environment
#   output_file  =    'dakota.log'# trailing comment
some_other_keyword
# full line comment error_file "dakota.err" # trailing comment
)";


BOOST_AUTO_TEST_CASE(test_valid_redirs)
{
  // TODO: Parameterized test
  std::vector<std::string> valid_inputs =
    { valid_redirs, valid_redirs_comments, valid_redirs_intervening_lines };
  for (const auto& input_text : valid_inputs) {
    std::string outfile, errfile;
    std::istringstream infile(input_text);
    Dakota::OutputManager::check_input_redirs(infile, outfile, errfile);

    BOOST_TEST(outfile == "dakota.log");
    BOOST_TEST(errfile == "dakota.err");
  }

  std::string outfile, errfile;
  std::istringstream infile(no_redirs_comments);
  Dakota::OutputManager::check_input_redirs(infile, outfile, errfile);

  BOOST_TEST(outfile == "");
  BOOST_TEST(errfile == "");
}


