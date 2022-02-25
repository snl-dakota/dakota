/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
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
  DakotaPlugins::EvalResponse
  evaluate(const DakotaPlugins::EvalRequest& request) override;
};


#endif
