/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "PluginInterface.hpp"
#include "ProblemDescDB.hpp"

#include <boost/dll/import.hpp>
#include <boost/filesystem.hpp>
#include <boost/version.hpp>

// Boost 1.76 and newer avoid the C++20 import keyword
// RATIONALE: Using preprocessor as isolated to this compilation unit
#if BOOST_VERSION >= 107600
#define dakota_boost_dll_import boost::dll::import_symbol
#else
#define dakota_boost_dll_import boost::dll::import
#endif


namespace Dakota {

void copy_gradient(int const fun_idx,
  std::vector<std::vector<double>> const& source,
  RealMatrix& dest) {
  int const dim = source[fun_idx].size();
  for (int i = 0; i < dim; ++i) {
    dest(i, fun_idx) = source[fun_idx][i];
  }
}

void copy_hessian(std::vector<std::vector<double>> const& source,
  RealSymMatrix& dest) {
  int const dim = dest.numRows();
  for (int i = 0; i < dim; ++i) {
    for (int j = 0; j <= i; ++j) {
      dest(i, j) = source[i][j];
    }
  }
}

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


// not sure active set passed in is consistent with response
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

void PluginInterface::derived_map_asynch(const ParamResponsePair& pair)
{
  // This interface only supports bulk synchronous batch evals for now
  // TODO: Catch this at construct time
  if (!batchEval) {
    Cerr << "\nError: Plugin interfaces support single or batch evaluations, "
	 << "but not\nasynchronous.\n";
    abort_handler(INTERFACE_ERROR);
  }
}


void PluginInterface::wait_local_evaluations(PRPQueue& prp_queue)
{
  // loading at first map to head off conflicting Python issues
  load_plugin();

  // prepare requests
  std::vector<DakotaPlugins::EvalRequest> plugin_requests;
  plugin_requests.reserve(prp_queue.size());
  for (const auto& prp : prp_queue)
    plugin_requests.push_back
      (form_eval_request(prp.variables(), prp.active_set(), prp.eval_id()));

  // evaluate
  const auto plugin_responses = pluginInterface->evaluate(plugin_requests);

  // retrieve responses; grr, need a std::zip or boost combine/tie
  auto pr_it = plugin_responses.begin();
  for (auto& prp : prp_queue) {
    // not sure why this indirection is needed to get a non-const Response&;
    // it's cheating by creating a non-const envelope
    Response resp = prp.response();
    populate_response(*pr_it, resp);
    completionSet.insert(prp.eval_id());
    ++pr_it;
  }
}


/** Load plugin if not already active */
void PluginInterface::load_plugin()
{
  if (pluginInterface) return;
  try {
#if BOOST_VERSION <= 107600
    pluginInterface = boost::dll::import<DakotaPlugins::DakotaInterfaceAPI>
#else
    pluginInterface = boost::dll::import_symbol<DakotaPlugins::DakotaInterfaceAPI>
#endif
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
  pluginInterface->set_analysis_drivers(analysisDrivers);
  pluginInterface->initialize();
}


DakotaPlugins::EvalRequest PluginInterface::form_eval_request
(const Variables& vars, const ActiveSet& set, int fn_eval_id) const
{
  DakotaPlugins::EvalRequest req;
  // TODO: do we want to use the legacy copy_data or another means?
  copy_data(vars.all_continuous_variables(), req.continuousVars);
  copy_data(vars.all_discrete_int_variables(), req.discreteIntVars);
  copy_data(vars.all_discrete_string_variables(), req.discreteStringVars);
  copy_data(vars.all_discrete_real_variables(), req.discreteRealVars);

  copy_data(vars.all_continuous_variable_labels(), req.continuousLabels);
  copy_data(vars.all_discrete_int_variable_labels(), req.discreteIntLabels);
  copy_data(vars.all_discrete_string_variable_labels(), req.discreteStringLabels);
  copy_data(vars.all_discrete_real_variable_labels(), req.discreteRealLabels);

  req.activeSet = set.request_vector();
  req.derivativeVars = set.derivative_vector();
  req.inputOrderedLabels = vars.ordered_labels();

  req.functionEvalId = fn_eval_id;

  return req;
}


// pass active set in here too
void PluginInterface::populate_response
(const DakotaPlugins::EvalResponse& plugin_response, Response& response) const
{
  auto const& asv = response.active_set_request_vector();

  // TODO: this is abuse, getting a reference to a non-const view...
  auto resp_fns = response.function_values_view();
  auto resp_gradients = response.function_gradients_view();
  auto resp_hessians = response.function_hessians_view();

  size_t const num_fns = response.num_functions();
  for (size_t i = 0; i < num_fns; ++i) {
    if (asv[i] & 1) {
      resp_fns[i] = plugin_response.functions[i];
    }
    if (asv[i] & 2) {
      copy_gradient(i, plugin_response.gradients, resp_gradients);
    }
    if (asv[i] & 4) {
      copy_hessian(plugin_response.hessians[i], resp_hessians[i]);
    }
  }

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
