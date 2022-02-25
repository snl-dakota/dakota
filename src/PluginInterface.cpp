/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "PluginInterface.hpp"
#include "ProblemDescDB.hpp"

#include <boost/dll/import.hpp>
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
  // loading at first map to head off conflicting Python issues
  load_plugin();

  // NOTE: May want to persist the request across input filter,
  // driver(s), output filter
  const DakotaPlugins::EvalRequest plugin_request =
    form_eval_request(vars, set, fn_eval_id);
  DakotaPlugins::EvalResponse plugin_response =
    pluginInterface->evaluate(plugin_request);
  populate_response(plugin_response, response);
}


void PluginInterface::wait_local_evaluations(PRPQueue& prp_queue)
{
  // loading at first map to head off conflicting Python issues
  load_plugin();
}

/** Load plugin if not already active */
void PluginInterface::load_plugin()
{
  if (pluginInterface) return;
  try {
    pluginInterface = boost::dll::import<DakotaPlugins::DakotaInterfaceAPI>
      (pluginPath,
	 "dakota_interface_plugin"  // name of the symbol to import
	 // TODO: append .dll, .so, .dylib via
	 //boost::dll::load_mode::append_decorations
	 //     boost::dll::load_mode::rtld_now
	 );
  }
  catch (const boost::system::system_error& e) {
    Cerr << "\nError: Could not load symbol dakota_interface_plugin from "
	 << "specified plugin \ninterface library '" << pluginPath
	 << "'\nDetails:\n"
	 << e.what() << std::endl;
    abort_handler(INTERFACE_ERROR);
  }
  if (outputLevel >= VERBOSE_OUTPUT)
    Cout << "Loading plugin interface from '" << pluginPath << "'" << std::endl;
  pluginInterface->initialize();
}


DakotaPlugins::EvalRequest PluginInterface::form_eval_request
(const Variables& vars, const ActiveSet& set, int fn_eval_id) const
{
  DakotaPlugins::EvalRequest req;
  // TODO: do we want to use the legacy copy_data or another means?
  copy_data(vars.all_continuous_variables(), req.continuousVars);

  return req;
}


void PluginInterface::populate_response
(const DakotaPlugins::EvalResponse plugin_response, Response& response) const
{
  // TODO: this is abuse, getting a reference to a non-const view...
  auto resp_fns = response.function_values_view();
  copy_data(plugin_response.functions, resp_fns);
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
