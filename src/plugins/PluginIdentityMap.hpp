/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_PLUGIN_IDENTITY_MAP_H
#define DAKOTA_PLUGIN_IDENTITY_MAP_H

#include "DakotaInterfaceAPI.hpp"


/** Demo plug-in that returns f_i(x) = x_i for all i */
class PluginIdentityMap: public DakotaPlugins::DakotaInterfaceAPI
{
public:
  DakotaPlugins::EvalResponse evaluate(
      DakotaPlugins::EvalRequest const& request) override;

private:
  void evaluate_functions(size_t const idx,
      DakotaPlugins::EvalRequest const& request,
      DakotaPlugins::EvalResponse& response);

  void evaluate_gradients(size_t const idx,
      DakotaPlugins::EvalRequest const& request,
      DakotaPlugins::EvalResponse& response);

  void evaluate_hessians(size_t const idx,
      DakotaPlugins::EvalRequest const& request,
      DakotaPlugins::EvalResponse& response);
};


#endif
