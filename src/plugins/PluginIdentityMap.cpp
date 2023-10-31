/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "PluginIdentityMap.hpp"
#include "dakota_symbol_visibility.hpp"

namespace DP = DakotaPlugins;

DP::EvalResponse PluginIdentityMap::evaluate(DP::EvalRequest const& request) {

  DP::EvalResponse response;
  resize_response_arrays(request, response);

  auto const& asv = request.activeSet;
  size_t const num_fns = asv.size();

  for (size_t i = 0; i < num_fns; ++i) {
    if (asv[i] & 1) {
      evaluate_functions(i, request, response);
    }
    if (asv[i] & 2) {
      evaluate_gradients(i, request, response);
    }
    if (asv[i] & 4) {
      evaluate_hessians(i, request, response);
    }
  }

  return response;

}

void PluginIdentityMap::evaluate_functions(size_t const idx,
    DP::EvalRequest const& request,
    DP::EvalResponse& response) {

  response.functions[idx] = request.continuousVars[idx];

}

void PluginIdentityMap::evaluate_gradients(size_t const idx,
    DP::EvalRequest const& request,
    DP::EvalResponse& response) {

  size_t const num_derivs = request.derivativeVars.size();

  for (size_t k = 0; k < num_derivs; ++k) {
    if (k == idx) {
      response.gradients[k][k] = 1.;
    } else {
      response.gradients[k][idx] = 0.;
    }
  }

}

void PluginIdentityMap::evaluate_hessians(size_t const idx,
    DP::EvalRequest const& request,
    DP::EvalResponse& response) {

  size_t const num_derivs = request.derivativeVars.size();

  // only filling in the lower triangular part
  for (size_t i = 0; i < num_derivs; ++i) {
    for (size_t j = 0; j <= i; ++j) {
      response.hessians[idx][i][j] = 0.;
    }
  }

}

extern "C" DAKOTA_SYMBOL_EXPORT PluginIdentityMap dakota_interface_plugin;
PluginIdentityMap dakota_interface_plugin;
