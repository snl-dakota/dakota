/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


/** \file redirect_regexs.cpp Verify regexs used to parse input file
    redirection options. */

#include "OutputManager.hpp"
#include "StudyConfig.hpp"

#include <gtest/gtest.h>

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

namespace {

std::string next_default_name(const std::string& resolved_name)
{
  const std::string prefix = "dakota_results";
  if (resolved_name == prefix)
    return prefix + ".2";

  const std::string dotted_prefix = prefix + ".";
  if (resolved_name.rfind(dotted_prefix, 0) != 0)
    throw std::runtime_error("Unexpected default results filename: " + resolved_name);

  const auto suffix = resolved_name.substr(dotted_prefix.size());
  const auto next_index = std::stoul(suffix) + 1;
  return dotted_prefix + std::to_string(next_index);
}

std::unique_ptr<Dakota::OutputManager> make_output_manager(const std::string& results_output_file = "dakota_results")
{
  auto mgr = std::make_unique<Dakota::OutputManager>();
  Dakota::StudyOutputConfig config;
  config.resultsOutput = true;
  config.resultsOutputFile = results_output_file;
  mgr->apply(config);
  return mgr;
}

} // namespace

TEST(redirect_regexs_tests, test_valid_redirs)
{
  // TODO: Parameterized test
  std::vector<std::string> valid_inputs =
    { valid_redirs, valid_spaceless_redirs, valid_redirs_comments,
      valid_redirs_intervening_lines };
  for (const auto& input_text : valid_inputs) {
    std::string outfile, errfile;
    std::istringstream infile(input_text);
    Dakota::OutputManager::check_input_redirs_impl(infile, outfile, errfile);

    EXPECT_TRUE((outfile == "dakota.log"));
    EXPECT_TRUE((errfile == "dakota.err"));
  }

  std::vector<std::string> invalid_inputs =
    { no_redirs_comments , no_redirs_adjacentword };
  for (const auto& input_text : invalid_inputs) {
    std::string outfile, errfile;
    std::istringstream infile(input_text);
    Dakota::OutputManager::check_input_redirs_impl(infile, outfile, errfile);

    EXPECT_TRUE((outfile == ""));
    EXPECT_TRUE((errfile == ""));
  }
}

TEST(output_manager_results_output_tests, auto_suffixes_default_results_output_files)
{
  auto first_mgr = make_output_manager();
  auto second_mgr = make_output_manager();

  const auto first_name = first_mgr->resolved_results_output_file();
  const auto second_name = second_mgr->resolved_results_output_file();

  EXPECT_EQ("dakota_results", first_name);
  EXPECT_EQ(next_default_name(first_name), second_name);
}

TEST(output_manager_results_output_tests, throws_on_duplicate_explicit_results_output_files)
{
  auto first_mgr = make_output_manager("explicit_results_output_test");
  EXPECT_EQ("explicit_results_output_test", first_mgr->resolved_results_output_file());

  auto second_mgr = make_output_manager("explicit_results_output_test");
  EXPECT_THROW(second_mgr->resolved_results_output_file(), std::runtime_error);
}

TEST(output_manager_results_output_tests, reuses_explicit_results_output_file_after_owner_destructs)
{
  {
    auto first_mgr = make_output_manager("reusable_explicit_results_output_test");
    EXPECT_EQ("reusable_explicit_results_output_test",
              first_mgr->resolved_results_output_file());
  }

  auto second_mgr = make_output_manager("reusable_explicit_results_output_test");
  EXPECT_EQ("reusable_explicit_results_output_test",
            second_mgr->resolved_results_output_file());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

