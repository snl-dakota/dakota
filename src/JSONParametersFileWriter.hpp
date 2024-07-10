/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include <string>
#include <vector>
#include "ParametersFileWriter.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Dakota {

class JSONParametersFileWriter : public ParametersFileWriter {
    public:
        
        /// Write a parameters file for a single evaluation
        void write_parameters_file(const Variables& vars, const ActiveSet& set,
                                        const Response& response, const std::string& prog,
                                        const std::vector<std::string>& an_comps,
                                        const std::string& full_eval_id,
                                        const std::string& params_fname) const override;

        /// Write a parameters file for a batch of evalulations
        void write_parameters_file(const PRPQueue& prp_queue,
                                        const std::string &prog,
                                        const std::vector<std::string>& an_comps,
                                        const std::string& eval_prefix_tag,
                                        int batch_id,
                                        const std::string & params_fname) const override;

    private:
        /// Helper for writing a single evaluation to JSON
        void write_evaluation_to_json(const Variables& vars,
                                        const ActiveSet& set,
                                        const Response& response,
                                        const std::string prog,
					const std::vector<std::string>& an_comps,
                                        std::string full_eval_id,
                                        json& json_obj) const;


};

}
