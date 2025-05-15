/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "JSONResultsFileReader.hpp"
#include "DakotaResponse.hpp"

namespace Dakota {

JSONResultsFileReader::JSONResultsFileReader(bool labeled) : ResultsFileReader(labeled) {}

void JSONResultsFileReader::read_results_file(Response& response, const std::filesystem::path &results_path, const int id) const {
  std::ifstream results_file(results_path);
    if (!results_file) {
        Cerr << "\nError: cannot open results file " << results_path
	    << " for evaluation " << std::to_string(id) << std::endl;
        abort_handler(INTERFACE_ERROR); // will clean up files unless file_save was specified
    }
    json j;
    try {
        // results_file >> j;
        j = json::parse(results_file);
    } catch(const json::parse_error& e) {
        throw FileReadException("Error(s) encountered reading results file " + results_path.string() +
			" for Evaluation " + std::to_string(id) + ":\n" + e.what());
    }
    results_file.close(); 
    try {
        response.read(j, false);
    }
    catch(const FileReadException& fr_except) {
        throw FileReadException("Error(s) encountered reading results file " + 
            results_path.string() + " for Evaluation " + 
            std::to_string(id) + ":\n" + fr_except.what()); 
    }
}

void JSONResultsFileReader::read_results_file(PRPQueue& prp_queue, const std::string &results_path,
		const int batch_id, IntSet &completion_set) const {

  std::ifstream results_file(results_path);
    if (!results_file) {
        Cerr << "\nError: cannot open results file " << results_path << 
            " for batch " << std::to_string(batch_id) << std::endl;
            abort_handler(INTERFACE_ERROR); // will clean up files unless file_save was specified
    }
    json j;
        try {
        // results_file >> j;
        j = json::parse(results_file);
    } catch(const json::parse_error& e) {
        throw FileReadException("Error(s) encountered reading results file " + results_path +
			" for Batch " + std::to_string(batch_id) + ":\n" + e.what());
    }
    results_file.close();
    size_t num_pairs = prp_queue.size();
    if(!j.is_array())
	throw ResultsFileError("Batch JSON results file is expected to contain an array at the top level");
    if(j.size() != num_pairs)
	throw ResultsFileError(std::string("Batch JSON results file expected to have ") + std::to_string(num_pairs)
			+ ", but the top-level array it contains has " + std::to_string(j.size()) + " elements");
    Response response;
    size_t eval_idx = 0;
    for(auto & pair : prp_queue) {
        response = pair.response();
	json& j_eval = j[eval_idx];
	// the read operation errors out for improperly formatted data
        try {
            response.read(j_eval, false);
        }
        catch(const FunctionEvalFailure & fneval_except) {
	    // must be re-thrown with context so that it can be caught in
	    // ProcessApplicInterface::wait_local_evaluation_batch(), which calls 
	    // ApplicationInterface::manage_failure().
            throw FunctionBatchEvalFailure(fneval_except.what(), pair, response);
        }
        catch(const FileReadException& fr_except) {
            throw FileReadException("Error(s) encountered reading batch results file " +
                    results_path + " for Evaluation " + std::to_string(pair.eval_id())
                    + ":\n" + fr_except.what()); 
        }
        completion_set.insert(pair.eval_id());
	++eval_idx;
    }
}
}
