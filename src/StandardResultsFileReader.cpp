/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "StandardResultsFileReader.hpp"

namespace Dakota {

StandardResultsFileReader::StandardResultsFileReader(bool labeled)
    : ResultsFileReader(labeled) {}

void StandardResultsFileReader::read_results_file(
    Response& response, const std::filesystem::path& results_path,
    const int id) const {
  std::ifstream recovery_stream(results_path);
  if (!recovery_stream) {
    Cerr << "\nError: cannot open results file " << results_path
         << " for evaluation " << std::to_string(id) << std::endl;
    abort_handler(
        INTERFACE_ERROR);  // will clean up files unless file_save was specified
  }
  try {
    response.read(recovery_stream, labeledFlag);
  } catch (const FileReadException& fr_except) {
    throw FileReadException("Error(s) encountered reading results file " +
                            results_path.string() + " for Evaluation " +
                            std::to_string(id) + ":\n" + fr_except.what());
  }
}

void StandardResultsFileReader::read_results_file(
    PRPQueue& prp_queue, const std::string& results_path, const int batch_id,
    IntSet& completion_set) const {
  std::ifstream results_file(results_path);
  if (!results_file) {
    Cerr << "\nError: cannot open results file " << results_path
         << " for batch " << std::to_string(batch_id) << std::endl;
    abort_handler(
        INTERFACE_ERROR);  // will clean up files unless file_save was specified
  }
  Response response;
  for (auto& pair : prp_queue) {
    std::stringstream eval_ss;
    while (true) {
      String eval_buffer;
      std::getline(results_file, eval_buffer);
      if (results_file.eof()) break;
      if (eval_buffer[0] == '#') {
        if (eval_ss.str().empty())
          continue;
        else
          break;
      } else {
        eval_ss << eval_buffer << std::endl;
      }
    }
    response = pair.response();
    // the read operation errors out for improperly formatted data
    try {
      response.read(eval_ss, labeledFlag);
    } catch (const FunctionEvalFailure& fneval_except) {
      // manage_failure(pair.variables(), response.active_set(), response,
      // pair.eval_id());
      throw;
    } catch (const FileReadException& fr_except) {
      throw FileReadException(
          "Error(s) encountered reading batch results file " + results_path +
          " for Evaluation " + std::to_string(pair.eval_id()) + ":\n" +
          fr_except.what());
    }
    completion_set.insert(pair.eval_id());
  }
  results_file.close();
}

}  // namespace Dakota
