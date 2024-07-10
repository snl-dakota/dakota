/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include "ResultsFileReader.hpp"

namespace Dakota {


class JSONResultsFileReader : public ResultsFileReader {
    public:
        JSONResultsFileReader(bool labeled);
	/// Read JSON-format results for a single evaluation
        void read_results_file(Response& response, const bfs::path &results_path, const int id) const override;
	/// Read JSON-format results for a batch of evaluations
        void read_results_file(PRPQueue& prp_queue, const std::string &results_path, const int batch_id, IntSet& completion_set) const override;
};

}
