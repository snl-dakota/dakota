/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "PluginInterface.hpp"
#include "ProblemDescDB.hpp"

#include <boost/filesystem.hpp>

namespace Dakota {

PluginInterface::PluginInterface(const ProblemDescDB& problem_db):
  ApplicationInterface(problem_db),
  pluginPath(problem_db.get_string("interface.plugin_library_path")),
  analysisDrivers(
    problem_db.get_sa("interface.application.analysis_drivers"))
{
  check_plugin_exists();
}


PluginInterface::~PluginInterface()
{
}


void PluginInterface::derived_map(const Variables& vars, const ActiveSet& set,
				  Response& response, int fn_eval_id)
{
}


void PluginInterface::wait_local_evaluations(PRPQueue& prp_queue)
{
}


void PluginInterface::load_plugin()
{
}


void PluginInterface::check_plugin_exists()
{
  // This only accounts for user-provided path case
  // TODO: locate plugin in default locations such as dakota/bin or dakota/lib
  if (!boost::filesystem::exists(pluginPath)) {
    Cerr << "\nError: Specified plugin interface library \n  '" << pluginPath
	 << "'\ndoes not exist.\n";
    abort_handler(INTERFACE_ERROR);
  }
}


}
