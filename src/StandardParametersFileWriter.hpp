/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include <string>
#include "ParametersFileWriter.hpp"

namespace Dakota {


class StandardParametersFileWriter : public ParametersFileWriter {
        
    private:
        /// Write parameters for single evaluation to open file
        void write_evaluation_to_stream(const Variables& vars,
                                        const ActiveSet& set,
                                        const Response& response,
                                        const std::vector<std::string>& an_comps,
                                        const StringArray& asv_labels, 
                                        const StringArray& dvv_labels,
                                        const StringArray& ac_labels,
                                        const StringArray& md_tags,
                                        std::string full_eval_id,
                                        std::ofstream& parameter_stream) const override;

};

}
