/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ProblemDescDBUtils.hpp"
#include "ProgramOptions.hpp"
#include "dakota_global_defs.hpp"

namespace Dakota {
namespace ProblemDescDBUtils {

/// @brief  Helper that reads an input file into a string
/// @param input_file path to the input file
/// @return string containing the contents of the file
std::string read_input_file(std::string_view input_file);

/// @brief Format and echo input contents to stdout
/// @param input_file 
/// @param template_flag 
void echo_input_helper(std::string_view input_string, bool template_flag);

std::pair<std::string, std::string> final_input_and_template(const ProgramOptions& prog_opts) {
    if ( !prog_opts.input_file().empty() && !prog_opts.input_string().empty() ) {
        Cerr << "\nError: final_input_and_template called with both input file and input "
            << "string." << std::endl;
        abort_handler(PARSE_ERROR);
    }
    std::string final_input{};
    std::string tmpl{};
    if(prog_opts.preproc_input()) {
        final_input = read_input_file(prog_opts.preprocessed_file());
        if(prog_opts.stdin_input()) {
            tmpl = prog_opts.input_string();
        } else {
            tmpl = read_input_file(prog_opts.input_file());
        }
    } else {
        if(prog_opts.stdin_input()) {
            final_input = prog_opts.input_string();
        } else {
            final_input = read_input_file(prog_opts.input_file());
        }
    }
    return std::make_pair(final_input, tmpl);
}

std::string read_input_file(std::string_view input_file) {
    std::ifstream file(input_file.data()); // Open the file
    if (!file) {
        Cerr << "\nError: Could not open input file '" << input_file
        << "' for reading." << std::endl;
        abort_handler(IO_ERROR);
    }

    std::stringstream buffer; // Create a stringstream to hold the file contents
    buffer << file.rdbuf();   // Read the file's contents into the stringstream

    return buffer.str(); // Convert the stringstream to a string and return it
}

void echo_input(std::string_view final_input, std::string_view template_string) {
    if (!template_string.empty())
            echo_input_helper(template_string, true /* is_template */);
    echo_input_helper(final_input, false /* not a template*/);
}

void echo_input_helper(std::string_view input_string, bool template_flag) {
    size_t header_len = (template_flag) ? 27 : 18;
    std::string header(header_len, '-');
    Cout << header << '\n';
    Cout << "Begin Dakota input";
    if(template_flag)
        Cout << " template\n";
    else
        Cout << "\n";
    Cout << header << std::endl;
    Cout << input_string << std::endl;
    Cout << "----------------\n";
    Cout << "End Dakota input\n";
    Cout << "----------------\n" << std::endl;
}

} // namespace ProblemDescDBUtils
} // namespace Dakota
