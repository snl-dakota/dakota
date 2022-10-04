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

std::string bum_comments = R"(
environment
output_file  =
  # change the log here:
  'dakota.log'# trailing comment
some_other_keyword
# full line comment
error_file "dakota.err" # trailing comment
)";


// check functions only with default ASV = 1's
BOOST_AUTO_TEST_CASE(test_valid_redirs)
{
  // TODO: Parameterized test
  std::vector<std::string> valid_inputs =
    { valid_redirs, valid_redirs_comments };
  //    { valid_redirs, valid_redirs_comments, bum_comments };
  for (const auto& input_text : valid_inputs) {
    std::string outfile, errfile;
    std::istringstream infile(input_text);
    Dakota::OutputManager::check_input_redirs(infile, outfile, errfile);

    BOOST_TEST(outfile == "dakota.log");
    BOOST_TEST(errfile == "dakota.err");
  }

}


