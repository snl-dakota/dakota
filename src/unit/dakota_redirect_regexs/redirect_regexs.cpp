/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/** \file redirect_regexs.cpp Verify regexs used to parse input file
    redirection options. */

#include <gtest/gtest.h>

#include "OutputManager.hpp"

// These examples should cause redirects
std::string valid_redirs = R"(
environment
output_file  =    'dakota.log'
some_other_keyword
  error_file "dakota.err"
)";

// apparently NIDR permits this
std::string valid_spaceless_redirs = R"(
environment
output_file'dakota.log'
some_other_keyword
error_file"dakota.err"
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

// Preceding word characters should not cause a redirect
std::string no_redirs_adjacentword = R"(
environment
results_output_file  =    'dakota.log'# trailing comment
  ANoutput_file  'dakota.log' 
some_other_keyword
# full line comment
  Berror_file "dakota.err" # trailing comment
  error_fileC "dakota.err" # trailing comment
)";

// This mess shouldn't cause a redirect
// TODO: Or should it? Not yet tested.
std::string no_redirs_junk = R"(
environment
output_file
   Hey, ho, there's some random content here
   = = =
   "oops, forgot a closing quote
   "noredir.log"  # here's my filename
error_file
   malformed content
   'noredir.err'
)";
// for syntax highlighting: '

TEST(redirect_regexs_tests, test_valid_redirs) {
  // TODO: Parameterized test
  std::vector<std::string> valid_inputs = {valid_redirs, valid_spaceless_redirs,
                                           valid_redirs_comments,
                                           valid_redirs_intervening_lines};
  for (const auto& input_text : valid_inputs) {
    std::string outfile, errfile;
    std::istringstream infile(input_text);
    Dakota::OutputManager::check_input_redirs_impl(infile, outfile, errfile);

    EXPECT_TRUE((outfile == "dakota.log"));
    EXPECT_TRUE((errfile == "dakota.err"));
  }

  std::vector<std::string> invalid_inputs = {no_redirs_comments,
                                             no_redirs_adjacentword};
  for (const auto& input_text : invalid_inputs) {
    std::string outfile, errfile;
    std::istringstream infile(input_text);
    Dakota::OutputManager::check_input_redirs_impl(infile, outfile, errfile);

    EXPECT_TRUE((outfile == ""));
    EXPECT_TRUE((errfile == ""));
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
