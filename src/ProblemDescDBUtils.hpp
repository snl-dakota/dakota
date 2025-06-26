/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once
#include <string_view>

namespace Dakota {

class ProgramOptions;
class ParallelLibrary;
class ProblemDescDB;
class UserModes;

namespace ProblemDescDBUtils {

/// @brief Return strings containing the final (preprocessed) input and
/// (possibly empty) template
/// @param prog_opts
/// @return The final (preprocessed, if necessary) input, read from file or
/// copied from string, and the template (possibly empty)
std::pair<std::string, std::string> final_input_and_template(
    const ProgramOptions& prog_opts);

/// @brief Format and echo the final (preprocessed) input and (if provided)
/// template strings to stdout
/// @param final_input final input that will be parsed
/// @param template_string possibly empty template string, prior to
/// preprocessing
void echo_input(std::string_view final_input, std::string_view template_string);

void check_and_broadcast_pdb(ProblemDescDB& problem_db,
                             const UserModes& user_modes,
                             ParallelLibrary& parallel_lib);

}  // namespace ProblemDescDBUtils
}  // namespace Dakota
