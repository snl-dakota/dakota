/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <string>
#include <vector>
#include "JSONParametersFileWriter.hpp"
#include "dakota_data_types.hpp"
#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>


using json = nlohmann::json;

namespace Dakota {

void JSONParametersFileWriter::write_parameters_file(const Variables& vars, const ActiveSet& set,
                                        const Response& response, const std::string& prog,
                                        const std::vector<std::string>& an_comps,
                                        const std::string& full_eval_id,
                                        const std::string& params_fname) const {
    std::ofstream parameter_stream(params_fname.c_str());
    if (!parameter_stream) {
        Cerr << "\nError: cannot create parameters file " << params_fname
             << std::endl;
        abort_handler(IO_ERROR);
    }
    std::string formatted_eval_id(full_eval_id);
    formatted_eval_id.erase(0,1); 
    boost::algorithm::replace_all(formatted_eval_id, String("."), String(":"));
    json json_out;
    write_evaluation_to_json(vars, set, response, prog, an_comps, formatted_eval_id, json_out);

    parameter_stream << std::setw(4) << json_out;
    parameter_stream.flush();
    parameter_stream.close();
}


        /// Write a parameters file for a batch of evalulations
void JSONParametersFileWriter::write_parameters_file(const PRPQueue& prp_queue,
                                        const std::string &prog,
                                        const std::vector<std::string>& an_comps,
                                        const std::string& eval_prefix_tag,
                                        int batch_id,
                                        const std::string & params_fname) const {
    std::ofstream parameter_stream(params_fname.c_str());

    if (!parameter_stream) {
        Cerr << "\nError: cannot create parameters file " << params_fname
             << std::endl;
        abort_handler(IO_ERROR);
    }

    auto first_pair = *prp_queue.begin();
    auto vars = first_pair.variables();
    auto set  = first_pair.active_set();
    auto response = first_pair.response();

    json json_out = json::array();
    std::string eval_and_batch = (eval_prefix_tag.empty()) ? std::to_string(batch_id) + ":" :
                                                             eval_prefix_tag + ":" + std::to_string(batch_id) + ":";
    for(const auto & pair : prp_queue) {
        int fn_eval_id = pair.eval_id();
        std::string full_eval_id = eval_and_batch + std::to_string(fn_eval_id);
        json single_eval;
        write_evaluation_to_json(pair.variables(), pair.active_set(), pair.response(), prog, an_comps, full_eval_id, single_eval);
        json_out.push_back(single_eval);
    }
    parameter_stream << std::setw(4) << json_out;
    parameter_stream.flush();
    parameter_stream.close();
}

void JSONParametersFileWriter::write_evaluation_to_json(const Variables& vars,
                                        const ActiveSet& set,
                                        const Response& response,
					const std::string prog,
                                        const std::vector<std::string>& an_comps,
                                        std::string full_eval_id,
                                        json& json_obj) const {

    const ShortArray&         asv         = set.request_vector();
    const SizetArray&         dvv         = set.derivative_vector();
    const StringArray&        resp_labels = response.function_labels();
    const StringArray& md_labels = response.shared_data().metadata_labels();
    size_t i, asv_len = asv.size(), dvv_len = dvv.size(),
        ac_len = an_comps.size(), md_len = md_labels.size();

    json_obj["variables"] = json::array();
    vars.write_json(json_obj["variables"]);

    json_obj["responses"] = json::array();
    for(size_t i = 0; i < asv_len; ++i) {
      json jsonfuncs = {
	      {"label" , resp_labels[i]},
	      {"active_set" , asv[i]}
      };
      json_obj["responses"].push_back(jsonfuncs);
    }
    json_obj["derivative_variables"] = dvv;
    json_obj["analysis_components"] = json::array();
    for(size_t i = 0; i < ac_len; ++i) {
	json json_an_comps = {
	   {"driver", prog},
           {"component", an_comps[i]}
        };
	json_obj["analysis_components"].push_back(json_an_comps);
    }
    json_obj["eval_id"] = full_eval_id;
    json_obj["metadata"] = md_labels;
}



}
