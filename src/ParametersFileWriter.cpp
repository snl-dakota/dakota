/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ParametersFileWriter.hpp"
#include "dakota_global_defs.hpp"
#include "dakota_data_types.hpp"
#include <memory>
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "DakotaActiveSet.hpp"
#include "StandardParametersFileWriter.hpp"
#include "ApreproParametersFileWriter.hpp"
#include "JSONParametersFileWriter.hpp"
#include <boost/algorithm/string.hpp>

namespace Dakota {


std::unique_ptr<ParametersFileWriter> ParametersFileWriter::get_writer(
    unsigned short params_file_format) {
        switch(params_file_format) {
            case PARAMETERS_FILE_STANDARD:
                return std::make_unique<StandardParametersFileWriter>();
                break;
            case PARAMETERS_FILE_APREPRO:
                return std::make_unique<ApreproParametersFileWriter>();
                break;
            case PARAMETERS_FILE_JSON:
                return std::make_unique<JSONParametersFileWriter>();
                break;
        }
}


void ParametersFileWriter::write_parameters_file(const PRPQueue& prp_queue,
                                        const std::string &prog,
                                        const std::vector<std::string>& an_comps,
                                        const std::string& eval_prefix_tag,
                                        int batch_id,
                                        const std::string & params_fname) const{
   
    std::ofstream parameter_stream(params_fname.c_str());

    using std::setw;
    if (!parameter_stream) {
        Cerr << "\nError: cannot create parameters file " << params_fname
             << std::endl;
        abort_handler(IO_ERROR);
    }

    auto first_pair = *prp_queue.begin();
    auto vars = first_pair.variables();
    auto set  = first_pair.active_set();
    auto response = first_pair.response();

    StringArray asv_labels, dvv_labels, ac_labels, md_tags;
    populate_labels(vars, set, response, prog, an_comps, asv_labels, dvv_labels, ac_labels, md_tags);

    std::string eval_and_batch = (eval_prefix_tag.empty()) ? std::to_string(batch_id) + ":" :
                                                            eval_prefix_tag + ":" + std::to_string(batch_id) + ":";    
    for(const auto & pair : prp_queue) {
        int fn_eval_id = pair.eval_id();
        std::string full_eval_id = eval_and_batch + std::to_string(fn_eval_id);
        write_evaluation_to_stream(pair.variables(), pair.active_set(), pair.response(), an_comps, asv_labels, dvv_labels, ac_labels, md_tags, full_eval_id, parameter_stream);                                            // appear in params file
    }
    parameter_stream.flush();
    parameter_stream.close();
}


void ParametersFileWriter::write_parameters_file(
    const Variables& vars, const ActiveSet& set, const Response& response,
    const std::string& prog, const std::vector<std::string>& an_comps, const std::string& full_eval_id,
    const std::string& params_fname) const {

    std::ofstream parameter_stream(params_fname.c_str());
    if (!parameter_stream) {
        Cerr << "\nError: cannot create parameters file " << params_fname
             << std::endl;
        abort_handler(IO_ERROR);
    }
    StringArray asv_labels, dvv_labels, ac_labels, md_tags;
    populate_labels(vars, set, response, prog, an_comps, asv_labels, dvv_labels, ac_labels, md_tags);
    std::string formatted_eval_id(full_eval_id);
    formatted_eval_id.erase(0,1); 
    boost::algorithm::replace_all(formatted_eval_id, String("."), String(":"));
    write_evaluation_to_stream(vars, set, response, an_comps, asv_labels, dvv_labels, ac_labels, md_tags, formatted_eval_id, parameter_stream);

    parameter_stream.flush();
    parameter_stream.close();
}


void ParametersFileWriter::write_evaluation_to_stream(const Variables& vars,
                                        const ActiveSet& set,
                                        const Response& response,
                                        const std::vector<std::string>& an_comps,
                                        const StringArray& asv_labels, 
                                        const StringArray& dvv_labels,
                                        const StringArray& ac_labels,
                                        const StringArray& md_tags,
                                        std::string full_eval_id,
                                        std::ofstream& parameter_stream) const {}


void ParametersFileWriter::populate_labels(const Variables& vars,
                            const ActiveSet& set,
                            const Response& response,
                            const std::string prog,
                            const std::vector<std::string>& an_comps,
                            StringArray& asv_labels,
                            StringArray& dvv_labels,
                            StringArray& ac_labels,
                            StringArray& md_tags) const {

    StringMultiArrayConstView acv_labels  = vars.all_continuous_variable_labels();
    SizetMultiArrayConstView  acv_ids     = vars.all_continuous_variable_ids();
    const ShortArray&         asv         = set.request_vector();
    const SizetArray&         dvv         = set.derivative_vector();
    const StringArray&        resp_labels = response.function_labels();
    const StringArray& md_labels = response.shared_data().metadata_labels();
    size_t i, asv_len = asv.size(), dvv_len = dvv.size(),
        ac_len = an_comps.size(), md_len = md_labels.size();
    
    asv_labels.resize(asv_len);
    dvv_labels.resize(dvv_len);
    ac_labels.resize(ac_len);
    md_tags.resize(md_len);
    
    build_labels(asv_labels, "ASV_");
    build_labels(dvv_labels, "DVV_");
    build_labels(ac_labels,  "AC_");
    build_labels(md_tags,  "MD_");
    for (i=0; i<asv_len; ++i)
        asv_labels[i] += ":" + resp_labels[i];
    for (i=0; i<dvv_len; ++i) {
        size_t acv_index = find_index(acv_ids, dvv[i]);
        if (acv_index != _NPOS)
            dvv_labels[i] += ":" + acv_labels[acv_index];
    }
    if (!prog.empty()) // empty string passed if multiple attributions possible
        for (i=0; i<ac_len; ++i)
            ac_labels[i] += ":" + prog; // attribution to particular program            
}

}
