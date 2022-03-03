/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "PluginIdentityMap.hpp"
#include "dakota_symbol_visibility.hpp"


namespace DP = DakotaPlugins;

DP::EvalResponse PluginIdentityMap::evaluate(const DP::EvalRequest& request)
{
  DP::EvalResponse resp;
  for (const auto& cv : request.continuousVars)
    resp.functions.push_back(cv);
  return resp;
}


extern "C" DAKOTA_SYMBOL_EXPORT PluginIdentityMap dakota_interface_plugin;
PluginIdentityMap dakota_interface_plugin;
