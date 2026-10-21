/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef OPT_TPL_TEST_FIXTURE_H
#define OPT_TPL_TEST_FIXTURE_H

#include "APPSOptimizer.hpp"

#include "LibraryEnvironment.hpp"
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "PluginSerialDirectApplicInterface.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp>
#include <nlohmann/json.hpp>


namespace Dakota {

namespace Opt_TPL_Test_Fixture {

  static Dakota::LibraryEnvironment * create_default_env(unsigned short method_name, bool finalize = true);

} // namespace Opt_TPL_Test_Fixture
} // namespace Dakota


//----------------------------------------------------------------

namespace {

  nlohmann::json default_opt_tpl_study_json(unsigned short method_name)
  {
    using json = nlohmann::json;

    json method_config;
    switch (method_name) {
    case Dakota::ASYNCH_PATTERN_SEARCH:
      method_config = json{{"asynch_pattern_search", json::object()}};
      break;
    case Dakota::OPTPP_PDS:
      method_config = json{{"optpp_pds", json::object()}};
      break;
    default:
      Cerr << "Error: unsupported method in default_opt_tpl_study_json()."
           << std::endl;
      Dakota::abort_handler(-1);
    }

    return json{
      {"method", json::array({method_config})},
      {"model", json::array({json::object()})},
      {"variables", json::array({{
        {"continuous_design", {
          {"count", 2},
          {"initial_point", {0.0, 0.0}}
        }}
      }})},
      {"interface", json::array({{
        {"analysis_drivers", {
          {"drivers", {"simple_quad"}},
          {"interface_type", {{"direct", json::object()}}}
        }}
      }})},
      {"responses", json::array({{
        {"response_type", {{"objective_functions", {{"count", 1}}}}},
        {"gradient_type", {{"analytic_gradients", true}}},
        {"hessian_type", {{"no_hessians", true}}}
      }})}
    };
  }

  class OptTestDirectApplicInterface: public Dakota::DirectApplicInterface
  {
    public:

      /// constructor
      OptTestDirectApplicInterface(const Dakota::ProblemDescDB& problem_db, Dakota::ParallelLibrary& parallel_lib):
        Dakota::DirectApplicInterface(problem_db, parallel_lib) { numAnalysisServers = 1;}

      /// destructor
      ~OptTestDirectApplicInterface() override { }

    protected:

      /// execute an analysis code portion of a direct evaluation invocation
      int derived_map_ac(const Dakota::String& ac_name) override
      {
        if (multiProcAnalysisFlag) {
          Cerr << "Error: plugin serial direct fn does not support multiprocessor "
            << "analyses." << std::endl;
          Dakota::abort_handler(-1);
        }

        int fail_code = 0;
        if (ac_name == "simple_quad") {
          Dakota::RealVector fn_grad;
          Dakota::RealSymMatrix fn_hess;
          if (directFnASV[0] & 2)
            fn_grad = Teuchos::getCol(Teuchos::View, fnGrads, 0);
          if (directFnASV[0] & 4)
            fn_hess = Dakota::RealSymMatrix(Teuchos::View, fnHessians[0],
                fnHessians[0].numRows());
          fail_code = simple_quad(xC, directFnASV[0], fnVals[0], fn_grad, fn_hess);
        }
        else {
          Cerr << ac_name << " is not available as an analysis within "
            << "OptTestDirectApplicInterface." << std::endl;
          Dakota::abort_handler(Dakota::INTERFACE_ERROR);
        }

        // Failure capturing
        if (fail_code) {
          std::string err_msg("Error evaluating plugin analysis_driver ");
          err_msg += ac_name;
          throw Dakota::FunctionEvalFailure(err_msg);
        }

        return 0;
      }


      /// evaluate the batch of jobs contained in prp_queue
      void wait_local_evaluations(Dakota::PRPQueue& prp_queue) override
      {
        if (multiProcAnalysisFlag) {
          Cerr << "Error: plugin serial direct fn does not support multiprocessor "
            << "analyses." << std::endl;
          Dakota::abort_handler(-1);
        }

        for (Dakota::PRPQueueIter prp_iter = prp_queue.begin();
            prp_iter != prp_queue.end(); prp_iter++) {
          // For each job in the processing queue, evaluate the response
          int fn_eval_id = prp_iter->eval_id();
          const Dakota::Variables& vars = prp_iter->variables();
          const Dakota::ActiveSet& set  = prp_iter->active_set();
          Dakota::Response         resp = prp_iter->response(); // shared rep
          if (outputLevel > Dakota::SILENT_OUTPUT)
            Cout << "SerialDirectApplicInterface:: evaluating function evaluation "
              << fn_eval_id << " in batch mode." << std::endl;
          Dakota::RealVector fn_grad; Dakota::RealSymMatrix fn_hess;
          short asv = set.request_vector()[0];
          Dakota::Real& fn_val = resp.function_value_view(0);
          if (asv & 2) fn_grad = resp.function_gradient_view(0);
          if (asv & 4) fn_hess = resp.function_hessian_view(0);
          simple_quad(vars.continuous_variables(), asv, fn_val, fn_grad, fn_hess);

          // indicate completion of job to ApplicationInterface schedulers
          completionSet.insert(fn_eval_id);
        }
      }

      /// invokes wait_local_evaluations() (no special nowait support)
      void test_local_evaluations(Dakota::PRPQueue& prp_queue) override
        { wait_local_evaluations(prp_queue); }

      /// no-op hides default run-time error checks at DirectApplicInterface level
      void set_communicators_checks(int max_eval_concurrency) override { }

    private:

      /// Simple Quadratic plug-in test function
      int simple_quad(const Dakota::RealVector& c_vars, short asv,
          Dakota::Real& fn_val, Dakota::RealVector& fn_grad,
          Dakota::RealSymMatrix& fn_hess)
      {
        if (c_vars.length() != 2) {
          Cerr << "Error: Bad number of variables in simple_quad direct fn."
            << std::endl;
          Dakota::abort_handler(Dakota::INTERFACE_ERROR);
        }

        Dakota::Real x1 = c_vars[0], x2 = c_vars[1];

        // **** f:
        if (asv & 1)
          fn_val = (x1+1.5)*(x1+1.5) + (x2-0.75)*(x2-0.75);

        // **** df/dx:
        if (asv & 2) {
          fn_grad[0] = 2.0*(x1+1.5);
          fn_grad[1] = 2.0*(x2-0.75);
        }

        // **** d^2f/dx^2:
        if (asv & 4) {
          Cerr << "Error: plugin serial direct fn does not support multiprocessor "
            << "analyses." << std::endl;
          Dakota::abort_handler(-1);
          //fn_hess(0,0) = 0.0;
          //fn_hess(0,1) = fnHessians[0](1,0) = 0.0;
          //fn_hess(1,1) = 0.0;
        }

        return 0;
      }
  };



  //----------------------------------------------------------------


  /** Simple plugin for a derived serial direct application interface
    instance. Memory ownership is transferred to the appropriate
    interface envelope in the environment. */
  void serial_interface_plugin(Dakota::LibraryEnvironment& env,
                               const std::string an_driver,
                               std::shared_ptr<Dakota::Interface> serial_iface)
  {
    std::string model_type(""); // demo: empty string will match any model type
    std::string interf_type("direct");

    bool plugged_in =
      env.plugin_interface(model_type, interf_type, an_driver, serial_iface);

    if (!plugged_in) {
      Cerr << "Error: no serial interface plugin performed.  Check "
        << "compatibility between parallel\n       configuration and "
        << "selected analysis_driver." << std::endl;
      Dakota::abort_handler(-1);
    }
  }
} // anonymous namespace


//----------------------------------------------------------------

static
Dakota::LibraryEnvironment * Dakota::Opt_TPL_Test_Fixture::create_default_env(unsigned short method_name, bool finalize)
{
  const auto study_json = default_opt_tpl_study_json(method_name);

  // Delay validation and construction so this helper keeps its existing
  // lifecycle while sourcing the study from JSON/IR instead of Data* objects.
  bool check_bcast_construct = false;

  Dakota::LibraryEnvironment * p_env =
    new Dakota::LibraryEnvironment(MPI_COMM_WORLD, study_json,
                                   check_bcast_construct);
  Dakota::LibraryEnvironment & env = *p_env;
  Dakota::ParallelLibrary& parallel_lib = env.parallel_library();

  // configure Dakota to throw a std::runtime_error instead of calling exit
  env.exit_mode("throw");

  if( !finalize )
    return p_env;

  // once done with changes: check database, broadcast, and construct iterators
  env.done_modifying_db();

  // plug the client's interface (function evaluator) into the Dakota environment
  Dakota::ProblemDescDB& problem_db = env.problem_description_db();
  std::shared_ptr<OptTestDirectApplicInterface> serial_iface(
    std::make_shared<OptTestDirectApplicInterface>(problem_db, parallel_lib));
  serial_interface_plugin(env, "simple_quad", serial_iface);

  return p_env;
}

#endif
