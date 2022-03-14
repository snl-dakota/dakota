/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_INTERFACE_API_H
#define DAKOTA_INTERFACE_API_H

#include <string>
#include <vector>

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

  void initialize() {};

  /// report the supported number of continuous, discrete int/string/real vars
  std::vector<size_t> variable_counts() { return std::vector<size_t>(); }

  /// report the supported variable labels
  /// TODO: container or input-spec order?
  std::vector<std::string> variable_labels()
  { return std::vector<std::string>(); }

  /// report the supported number of response functions
  size_t functions() { return 0; }

  /// report the supported function labels
  std::vector<std::string> function_labels()
  { return std::vector<std::string>(); }

  /// single evaluator
  virtual EvalResponse evaluate(EvalRequest const& request) = 0;

  /// batch evaluator; default implementation delegates to single evaluate
  std::vector<EvalResponse>
  evaluate(std::vector<EvalRequest> const& requests);

  void finalize() {};

protected:

  void resize_response_arrays(
      EvalRequest const& request,
      EvalResponse& response) {

    size_t const num_fns = request.activeSet.size();
    size_t const num_derivs = request.derivativeVars.size();

    response.functions.resize(num_fns);
    response.gradients.resize(num_derivs);
    response.hessians.resize(num_fns);

    for (size_t k = 0; k < num_derivs; ++k) {
      response.gradients[k].resize(num_fns);
    }

    for (size_t i = 0; i < num_fns; ++i) {
      response.hessians[i].resize(num_derivs);
      for (size_t j = 0; j < num_derivs; ++j) {
        response.hessians[i][j].resize(num_derivs);
      }
    }

  }

};

inline std::vector<EvalResponse>
DakotaInterfaceAPI::evaluate(std::vector<EvalRequest> const& requests)
{
  std::vector<EvalResponse> responses;
  size_t const num_requests = requests.size();
  responses.resize(num_requests);
  for (size_t i = 0; i < num_requests; ++i) {
    responses[i] = evaluate(requests[i]);
  }

  return responses;
}

}

#endif
