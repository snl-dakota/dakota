/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_PLUGIN_INTERFACE_H
#define DAKOTA_PLUGIN_INTERFACE_H

#include "ApplicationInterface.hpp"
#include "plugins/DakotaInterfaceAPI.hpp"

#include <boost/shared_ptr.hpp> // blech


namespace Dakota {

class PluginInterface: public ApplicationInterface
{

public:

  PluginInterface(const ProblemDescDB& problem_db);

  ~PluginInterface();
  
  void derived_map(const Variables& vars, const ActiveSet& set,
		   Response& response, int fn_eval_id);

  void derived_map_asynch(const ParamResponsePair& pair);

  /// For plugins, implements blocking bulk-synchronous evaluation of
  /// batch (PRPQueue)
  void wait_local_evaluations(PRPQueue& prp_queue);


protected:

  /// Use Boost DLL to runtime load the plugin
  void load_plugin();

  /// map variables and set to the plugin request
  DakotaPlugins::EvalRequest form_eval_request
  (const Variables& vars, const ActiveSet& set, int fn_eval_id) const;

  /// map plugin response to Dakota response
  void populate_response
  (const DakotaPlugins::EvalResponse& plugin_response, Response& response) const;

  /// path to the plugin to load, e.g., /path/to/libuser_plugin.so
  String pluginPath;

  /// the interface class loaded via plugin
  boost::shared_ptr<DakotaPlugins::DakotaInterfaceAPI> pluginInterface;


  /// list of drivers to perform core simulation mappings (can
  /// potentially be executed concurrently via MPI)
  StringArray analysisDrivers;

private:

  /// validate that the plugin exists on the filesystem
  void check_plugin_exists();

};

}

#endif
