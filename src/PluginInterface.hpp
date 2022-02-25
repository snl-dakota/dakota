/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_PLUGIN_INTERFACE_H
#define DAKOTA_PLUGIN_INTERFACE_H

#include "ApplicationInterface.hpp"

namespace Dakota {

class PluginInterface: public ApplicationInterface
{

public:

  PluginInterface(const ProblemDescDB& problem_db);

  ~PluginInterface();
  
  void derived_map(const Variables& vars, const ActiveSet& set,
		   Response& response, int fn_eval_id);

  /// For plugins, implements blocking bulk-synchronous evaluation of
  /// batch (PRPQueue)
  void wait_local_evaluations(PRPQueue& prp_queue);


protected:

  /// Use Boost DLL to runtime load the plugin
  void load_plugin();

  /// path to the plugin to load, e.g., /path/to/libuser_plugin.so
  String pluginPath;

  /// list of drivers to perform core simulation mappings (can
  /// potentially be executed concurrently via MPI)
  StringArray analysisDrivers;

private:

  /// validate that the plugin exists on the filesystem
  void check_plugin_exists();

};

}

#endif
