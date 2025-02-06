/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_INTERFACE_API_H
#define DAKOTA_INTERFACE_API_H

#include <string>
#include <vector>
#include <iostream>

namespace DakotaPlugins {

/** Data to send an evaluation request to the interface plugin API */
class EvalRequest {

public:
  std::vector<double> continuousVars;
  std::vector<int> discreteIntVars;
  std::vector<std::string> discreteStringVars;
  std::vector<double> discreteRealVars;

  std::vector<short> activeSet;
  /// 1-based IDs of derivative variables
  std::vector<size_t> derivativeVars;

  // maybe be able to share these among requests in the batch case
  std::vector<std::string> continuousLabels;
  std::vector<std::string> discreteIntLabels;
  std::vector<std::string> discreteStringLabels;
  std::vector<std::string> discreteRealLabels;
  std::vector<std::string> inputOrderedLabels;

  int functionEvalId = -1;

};


/** Data to return an evaluation request from the interface plugin API */
class EvalResponse {

public:
  std::vector<double> functions;
  std::vector<std::vector<double>> gradients;
  std::vector<std::vector<std::vector<double>>> hessians;

};


/** API for Dakota plugin Interfaces. Only std c++ allowed as
    specializations must be able to compile without Dakota.
 */
class DakotaInterfaceAPI
{

public:

  std::vector<std::string> analysisDrivers;

  virtual void initialize() {};

  /// set the analysis drivers from the input file
  void set_analysis_drivers(std::vector<std::string> const& analysis_drivers) {
    analysisDrivers = analysis_drivers;
  }

  /// report the supported number of continuous, discrete int/string/real vars
  virtual std::vector<size_t> variable_counts()
    { return std::vector<size_t>(); }

  /// report the supported variable labels
  /// TODO: container or input-spec order?
  virtual std::vector<std::string> variable_labels()
    { return std::vector<std::string>(); }

  /// report the supported number of response functions
  virtual size_t functions() { return 0; }

  /// report the supported function labels
  std::vector<std::string> function_labels()
    { return std::vector<std::string>(); }

  /// single evaluator
  virtual EvalResponse evaluate(EvalRequest const& request) = 0;

  // DTS: add batch ID as argument or put into request object?
  /// batch evaluator; default implementation delegates to single evaluate
  virtual std::vector<EvalResponse>
      evaluate(std::vector<EvalRequest> const& requests) {
    std::vector<EvalResponse> responses;
    size_t const num_requests = requests.size();
    responses.resize(num_requests);
    for (size_t i = 0; i < num_requests; ++i) {
      responses[i] = evaluate(requests[i]);
    }
    return responses;
  }

  virtual void finalize() {};

protected:

  void resize_response_arrays(
      EvalRequest const& request,
      EvalResponse& response) {

    size_t const num_fns = request.activeSet.size();
    size_t const num_derivs = request.derivativeVars.size();

    response.functions.resize(num_fns);
    response.gradients.resize(num_fns);
    response.hessians.resize(num_fns);

    for (size_t i = 0; i < num_fns; ++i) {
      response.gradients[i].resize(num_derivs);
      response.hessians[i].resize(num_derivs);
      for (size_t j = 0; j < num_derivs; ++j) {
        response.hessians[i][j].resize(num_derivs);
      }
    }
  }

};

}

#endif
