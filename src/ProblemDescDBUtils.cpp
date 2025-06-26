/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ProblemDescDBUtils.hpp"

#include "ParallelLibrary.hpp"
#include "ProblemDescDB.hpp"
#include "ProgramOptions.hpp"
#include "UserModes.hpp"
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

std::pair<std::string, std::string> final_input_and_template(
    const ProgramOptions& prog_opts) {
  if (!prog_opts.input_file().empty() && !prog_opts.input_string().empty()) {
    Cerr << "\nError: final_input_and_template called with both input file and "
            "input "
         << "string." << std::endl;
    abort_handler(PARSE_ERROR);
  }
  std::string final_input{};
  std::string tmpl{};
  if (prog_opts.preproc_input()) {
    final_input = read_input_file(prog_opts.preprocessed_file());
    if (!prog_opts.input_string().empty()) {
      tmpl = prog_opts.input_string();
    } else {
      tmpl = read_input_file(prog_opts.input_file());
    }
  } else {
    if (!prog_opts.input_string().empty()) {
      final_input = prog_opts.input_string();
    } else {
      final_input = read_input_file(prog_opts.input_file());
    }
  }
  return std::make_pair(final_input, tmpl);
}

std::string read_input_file(std::string_view input_file) {
  std::ifstream file(input_file.data());  // Open the file
  if (!file) {
    Cerr << "\nError: Could not open input file '" << input_file
         << "' for reading." << std::endl;
    abort_handler(IO_ERROR);
  }

  std::stringstream buffer;  // Create a stringstream to hold the file contents
  buffer << file.rdbuf();    // Read the file's contents into the stringstream

  return buffer.str();  // Convert the stringstream to a string and return it
}

void echo_input(std::string_view final_input,
                std::string_view template_string) {
  if (!template_string.empty())
    echo_input_helper(template_string, true /* is_template */);
  echo_input_helper(final_input, false /* not a template*/);
}

void echo_input_helper(std::string_view input_string, bool template_flag) {
  size_t header_len = (template_flag) ? 27 : 18;
  std::string header(header_len, '-');
  Cout << header << '\n';
  Cout << "Begin Dakota input";
  if (template_flag)
    Cout << " template\n";
  else
    Cout << "\n";
  Cout << header << std::endl;
  Cout << input_string << std::endl;
  Cout << "----------------\n";
  Cout << "End Dakota input\n";
  Cout << "----------------\n" << std::endl;
}

void check_and_broadcast_pdb(ProblemDescDB& problem_db,
                             const UserModes& user_modes,
                             ParallelLibrary& parallel_lib) {
  auto rep = problem_db.get_rep();
  // Check to make sure at least one of each of the keywords was found
  // in the problem specification file; checks only happen on Dakota rank 0
  if (parallel_lib.world_rank() == 0) problem_db.check_input(user_modes);

  // bcast a minimal MPI buffer containing the input specification
  // data prior to post-processing

  // DAKOTA's old design for reading the input file was for world rank 0 to
  // get the input filename from cmd_line_handler (after MPI_Init) and bcast
  // the character buffer to all other processors (having every processor
  // query the cmd_line_handler was failing because of the effect of MPI_Init
  // on argc and argv).  Then every processor yyparsed.  This worked fine but
  // was not scalable for MP machines with a limited number of I/O devices.

  // Now, world rank 0 yyparse's and sends all the parsed data in a single
  // buffer to all other ranks.
  if (parallel_lib.world_size() > 1) {
    if (parallel_lib.world_rank() == 0) {
      problem_db.enforce_unique_ids();
      problem_db.derived_broadcast();  // pre-processor
      MPIPackBuffer send_buffer;
      send_buffer << rep->environmentSpec << rep->dataMethodList
                  << rep->dataModelList << rep->dataVariablesList
                  << rep->dataInterfaceList << rep->dataResponsesList;

      // Broadcast length of buffer so that servers can allocate MPIUnpackBuffer
      int buffer_len = send_buffer.size();
      parallel_lib.bcast_w(buffer_len);

      // Broadcast actual buffer
      parallel_lib.bcast_w(send_buffer);
#ifdef MPI_DEBUG
      Cout << "DB buffer to send on world rank " << parallel_lib.world_rank()
           << ":\n"
           << rep->environmentSpec << rep->dataMethodList
           << rep->dataVariablesList << rep->dataInterfaceList
           << rep->dataResponsesList << std::endl;
#endif  // MPI_DEBUG
    } else {
      // receive length of incoming buffer and allocate space for
      // MPIUnpackBuffer
      int buffer_len;
      parallel_lib.bcast_w(buffer_len);

      // receive incoming buffer
      MPIUnpackBuffer recv_buffer(buffer_len);
      parallel_lib.bcast_w(recv_buffer);
      recv_buffer >> rep->environmentSpec >> rep->dataMethodList >>
          rep->dataModelList >> rep->dataVariablesList >>
          rep->dataInterfaceList >> rep->dataResponsesList;
#ifdef MPI_DEBUG
      Cout << "DB buffer received on world rank " << parallel_lib.world_rank()
           << ":\n"
           << rep->environmentSpec << rep->dataMethodList
           << rep->dataVariablesList << rep->dataInterfaceList
           << rep->dataResponsesList << std::endl;
#endif  // MPI_DEBUG
        // derived_broadcast(); // post-processor
    }
  } else {
#ifdef DEBUG
    Cout << "DB parsed data:\n"
         << rep->environmentSpec << rep->dataMethodList
         << rep->dataVariablesList << rep->dataInterfaceList
         << rep->dataResponsesList << std::endl;
#endif  // DEBUG
    problem_db.enforce_unique_ids();
    problem_db.derived_broadcast();
  }

  // After broadcast, perform post-processing on all processors to
  // size default variables/responses specification vectors (avoid
  // sending large vectors over an MPI buffer).
  problem_db.post_process();
}

}  // namespace ProblemDescDBUtils
}  // namespace Dakota
