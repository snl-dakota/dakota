/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include<nlohmann/json.hpp>
#include <string>
#include "dakota_data_types.hpp"

using json = nlohmann::json;

namespace Dakota {

class RespMetaDataT;

class JSONResultsParserError : public std::runtime_error {
    public:
    JSONResultsParserError(const std::string& msg) : std::runtime_error(msg) {}
};

class JSONResultsParser {
    public:
        JSONResultsParser(const json& j);
	bool failed() const;
        double metadata(const std::string& label) const;
        double function(const std::string& label) const;
        RealVector gradient(const std::string& label) const;
	RealSymMatrix hessian(const std::string& label) const;
    private:
	const json& jref; 
        bool failedFlag;
	bool hasFunctions;
	bool hasGradients;
	bool hasHessians;
	bool hasMetadata;
};

}
