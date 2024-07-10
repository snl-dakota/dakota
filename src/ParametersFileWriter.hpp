/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include <memory>
#include <vector>
#include <string>
#include "dakota_data_types.hpp"
#include "PRPMultiIndex.hpp"

namespace Dakota {

class Variables;
class ActiveSet;
class Response;


class ParametersFileWriter {
    public:
        /// Return pointer to derived class
        static std::unique_ptr<ParametersFileWriter> get_writer(unsigned short params_file_format);

        /// Write a parameters file for a single evaluation
        virtual void write_parameters_file(const Variables& vars, const ActiveSet& set,
                                        const Response& response, const std::string& prog,
                                        const std::vector<std::string>& an_comps,
                                        const std::string& full_eval_id,
                                        const std::string& params_fname) const;

        /// Write a parameters file for a batch of evalulations
        virtual void write_parameters_file(const PRPQueue& prp_queue,
                                        const std::string &prog,
                                        const std::vector<std::string>& an_comps,
                                        const std::string& eval_prefix_tag,
                                        int batch_id,
                                        const std::string & params_fname) const;
    protected:

        /// Write individual parameter set to stream
        virtual void write_evaluation_to_stream(const Variables& vars,
                                        const ActiveSet& set,
                                        const Response& response,
                                        const std::vector<std::string>& an_comps,
                                        const StringArray& asv_labels, 
                                        const StringArray& dvv_labels,
                                        const StringArray& ac_labels,
                                        const StringArray& md_tags,
                                        const std::string full_eval_id,
                                        std::ofstream& parameter_stream) const;

        /// create labels for use in aprepro and dakota format parameters files
        void populate_labels(const Variables& vars,
                            const ActiveSet& set,
                            const Response& response,
                            const std::string prog,
                            const std::vector<std::string>& an_comps,
                            StringArray& asv_labels,
                            StringArray& dvv_labels,
                            StringArray& ac_labels,
                            StringArray& md_tags) const;

};

}
