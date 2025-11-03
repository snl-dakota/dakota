/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "JSONResultsParser.hpp"
#include "JSONUtils.hpp"
#include <iostream>
#include <string>

namespace Dakota {

JSONResultsParser::JSONResultsParser(const json& j) : jref(j), failedFlag(false) {
    if(!jref.is_object())
        throw JSONResultsParserError("JSON results is not an object");
    if(jref.contains("fail") && jref["fail"].is_boolean()) {
	failedFlag = jref["fail"].template get<bool>();
    }
    hasFunctions = jref.contains("functions") && jref["functions"].is_object();
    hasGradients = jref.contains("gradients") && jref["gradients"].is_object();
    hasHessians = jref.contains("hessians") && jref["hessians"].is_object();
    hasMetadata = jref.contains("metadata") && jref["metadata"].is_object();
}

bool JSONResultsParser::failed() const {
    return failedFlag;
}

double JSONResultsParser::metadata(const std::string& label) const {
   if(!hasMetadata)
        throw JSONResultsParserError("JSON results object does not contain 'metadata', or it's not an object");
   if(!jref["metadata"].contains(label))
        throw JSONResultsParserError(std::string("'metadata' object in JSON results object does not contain '") +
                                     label + "'");
    double result;
    try {
        result = jref["metadata"][label].template get<JSONDoubleElement>().value;
    } catch(const nlohmann::detail::type_error& e) {
	throw JSONResultsParserError(std::string("In JSON results object, metadata value for '") 
				   + label + "' could not be converted to a number");
    } catch(const std::invalid_argument &e) {
        			throw JSONResultsParserError(std::string("In JSON results object, metadata value for '") 
				   + label + "' could not be converted to a number");
    }
    return result;
}

double JSONResultsParser::function(const std::string& label) const {
   if(!hasFunctions)
        throw JSONResultsParserError("JSON results object does not contain 'functions', or it's not an object");
   if(!jref["functions"].contains(label))
        throw JSONResultsParserError(std::string("'functions' object in JSON results object does not contain '") +
                                     label + "'");
    double result;
    try {
        result = jref["functions"][label].template get<JSONDoubleElement>().value;
    } catch(const nlohmann::detail::type_error& e) {
	   throw JSONResultsParserError(std::string("In JSON results object, function value for '") 
				   + label + "' could not be converted to a number");
    } catch(const std::invalid_argument &e) {
	   throw JSONResultsParserError(std::string("In JSON results object, function value for '") 
				   + label + "' could not be converted to a number");
    }

    return result;
}

RealVector JSONResultsParser::gradient(const std::string& label) const {
    if(!hasGradients)
         throw JSONResultsParserError("JSON results object does not contain 'gradients', or it's not an object");
    if(!jref["gradients"].contains(label))
         throw JSONResultsParserError(std::string("'gradients' object in JSON results object does not contain '") +
    			 label + "'");
    RealVector grad;
    try {
        grad = jref["gradients"][label].template get<JSONRealVector>().value;
    } catch(const nlohmann::detail::type_error& e) {
            throw JSONResultsParserError(std::string("In JSON results object, gradient for '") + label + "' contains " +
				   "an element that could not be converted to a number");
    } catch(const std::invalid_argument &e) {
            throw JSONResultsParserError(std::string("In JSON results object, gradient for '") + label + "' contains " +
				   "an element that could not be converted to a number");
    } catch(const JSONStoreError &e) {
         throw JSONResultsParserError(std::string("In JSON results object, gradient for '") + label + "' is not an array");   
    }

    return grad;
}

RealSymMatrix JSONResultsParser::hessian(const std::string& label) const {
    if(!hasHessians)
        throw JSONResultsParserError("JSON results object does not contain 'hessians', or it's not an object");
    if(!jref["hessians"].contains(label))
         throw JSONResultsParserError(std::string("'hessians' object in JSON results object does not contain '") +
                                     label + "'");
    const auto & jhess = jref["hessians"][label];
    RealSymMatrix hessian;
    try {
        hessian = jhess.template get<JSONRealSymMatrix>().value;
    } catch(const nlohmann::detail::type_error& e) {
        throw JSONResultsParserError(std::string("In JSON results object, Hessian for '") + 
		   label + "' contains an element that could not be converted to a number");
    } catch(const std::invalid_argument &e) {
        throw JSONResultsParserError(std::string("In JSON results object, Hessian for '") + 
                   label + "' contains an element that could not be converted to a number");
    } catch(const JSONStoreError &e) {
        const std::string msg(e.what());
	if(msg == "expected array object")
            throw JSONResultsParserError(std::string("In JSON results object, Hessian for '") + label + "' is not an array");
	if(msg == "expected element to be an array object")
            throw JSONResultsParserError(std::string("In JSON results object, Hessian for '") + label +
			    "' contains a row that is not an array");
        if(msg == "matrix is not square")
            throw JSONResultsParserError(std::string("In JSON results object, Hessian for '") + label +
			    "' is not square");
    }

    return hessian;
}
} // dakota namespace
