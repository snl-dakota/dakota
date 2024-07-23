#include "JSONResultsParser.hpp"
#include <iostream>

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
        result = jref["metadata"][label].template get<double>();
    } catch(const nlohmann::detail::type_error& e) {
	   throw JSONResultsParserError(std::string("In JSON results object, metadata value for '") 
				   + label + "' is non-numeric");
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
        result = jref["functions"][label].template get<double>();
    } catch(const nlohmann::detail::type_error& e) {
	   throw JSONResultsParserError(std::string("In JSON results object, function value for '") 
				   + label + "' is non-numeric");
    }
    return result;
}

RealVector JSONResultsParser::gradient(const std::string& label) const {
    if(!hasGradients)
         throw JSONResultsParserError("JSON results object does not contain 'gradients', or it's not an object");
    if(!jref["gradients"].contains(label))
         throw JSONResultsParserError(std::string("'gradients' object in JSON results object does not contain '") +
                                     label + "'");
    if(!jref["gradients"][label].is_array())
         throw JSONResultsParserError(std::string("In JSON results object, gradient for '") + label + "' is not an array");   
    auto n = jref["gradients"][label].size();
    RealVector grad(n);
    for(int i = 0; i < n; ++i) {
        try {
            grad[i] = jref["gradients"][label][i].template get<double>();
	} catch(const nlohmann::detail::type_error& e) {
            throw JSONResultsParserError(std::string("In JSON results object, gradient for '") + label + "' contains " +
			    "a non-numeric entry");
	}
    }
    return grad;
}

RealSymMatrix JSONResultsParser::hessian(const std::string& label) const {
    if(!hasHessians)
         throw JSONResultsParserError("JSON results object does not contain 'hessians', or it's not an object");
    if(!jref["hessians"].contains(label))
         throw JSONResultsParserError(std::string("'hessians' object in JSON results object does not contain '") +
                                     label + "'");
    if(!jref["hessians"][label].is_array())
         throw JSONResultsParserError(std::string("In JSON results object, Hessian for '") + label + "' is not an array");  
    const auto& jhess = jref["hessians"][label];
    auto n = jhess.size();
    for(const auto &row : jhess) {
        if(!row.is_array())
	    throw JSONResultsParserError(std::string("In JSON results object, Hessian for '") + label +
			    "' contains a row that is not an array");
	if(row.size() != n)
            throw JSONResultsParserError(std::string("In JSON results object, Hessian for '") + label +
			    "' is not square");
    }
    RealSymMatrix hessian(n);
    for(int i = 0; i < n; ++i) {
        for(int j = 0; j <= i; ++j) {
	    try {
                hessian(i, j) = jhess[i][j].template get<double>();
	    } catch(const nlohmann::detail::type_error& e) {
                 throw JSONResultsParserError(std::string("In JSON results object, Hessian for '") + 
				 label + "' contains a non-numeric entry");
	    }
	}
    }
    return hessian;
}
}
