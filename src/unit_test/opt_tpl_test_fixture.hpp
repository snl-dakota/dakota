#ifndef OPT_TPL_TEST_FIXTURE_H
#define OPT_TPL_TEST_FIXTURE_H

#include "APPSOptimizer.hpp"

#include "ParallelLibrary.hpp"
#include "ProblemDescDB.hpp"
#include "LibraryEnvironment.hpp"
#include "DakotaModel.hpp"
#include "DakotaInterface.hpp"
#include "PluginSerialDirectApplicInterface.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


namespace Dakota {

namespace Opt_TPL_Test_Fixture {

  static Dakota::LibraryEnvironment * create_default_env(unsigned short method_name, bool finalize = true);

} // namespace Opt_TPL_Test_Fixture
} // namespace Dakota


//----------------------------------------------------------------

namespace {

  class OptTestDirectApplicInterface: public Dakota::DirectApplicInterface
  {
    public:

      /// constructor
      OptTestDirectApplicInterface(const Dakota::ProblemDescDB& problem_db):
        Dakota::DirectApplicInterface(problem_db) { }

      /// destructor
      ~OptTestDirectApplicInterface() { }

    protected:

      /// execute an analysis code portion of a direct evaluation invocation
      int derived_map_ac(const Dakota::String& ac_name)
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
      void wait_local_evaluations(Dakota::PRPQueue& prp_queue)
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
      void test_local_evaluations(Dakota::PRPQueue& prp_queue)
        { wait_local_evaluations(prp_queue); }

      /// no-op hides default run-time error checks at DirectApplicInterface level
      void set_communicators_checks(int max_eval_concurrency) { }

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


  /** Simple plugin for a derived serial direct application
    interface instance */
  void serial_interface_plugin(Dakota::LibraryEnvironment& env)
  {
    std::string model_type(""); // demo: empty string will match any model type
    std::string interf_type("direct");
    std::string an_driver("simple_quad");

    Dakota::ProblemDescDB& problem_db = env.problem_description_db();
    Dakota::Interface* serial_iface = 
      new OptTestDirectApplicInterface(problem_db);

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
  // No input file set --> no parsing.  Could set other command line
  // options such as restart in opts:
  Dakota::ProgramOptions opts;
  opts.echo_input(false);

  // delay validation/sync of the Dakota database and iterator
  // construction to allow update after all data is populated
  bool check_bcast_construct = false;

  // set up a Dakota instance
  Dakota::LibraryEnvironment * p_env = new Dakota::LibraryEnvironment(MPI_COMM_WORLD, opts, check_bcast_construct);
  Dakota::LibraryEnvironment & env = *p_env;
  Dakota::ParallelLibrary& parallel_lib = env.parallel_library();

  // configure Dakota to throw a std::runtime_error instead of calling exit
  env.exit_mode("throw");

  // Now set the various data to specify the Dakota study
  Dakota::DataMethod   dme; Dakota::DataModel    dmo;
  Dakota::DataVariables dv; Dakota::DataInterface di; Dakota::DataResponses dr;
  if (parallel_lib.world_rank() == 0) {
    // This version uses direct Data instance population.  Initial instantiation
    // populates all the defaults.  Default Environment and Model data are used.
    Dakota::DataMethodRep*    dmr = dme.data_rep();
    Dakota::DataVariablesRep* dvr =  dv.data_rep();
    Dakota::DataInterfaceRep* dir =  di.data_rep();
    Dakota::DataResponsesRep* drr =  dr.data_rep();
    // Set any non-default values: mimic default_input
    dmr->methodOutput = SILENT_OUTPUT;
    dmr->methodName = method_name;
    dvr->numContinuousDesVars = 2;
    dir->interfaceType = Dakota::TEST_INTERFACE;
    dir->analysisDrivers.push_back("simple_quad");
    drr->numObjectiveFunctions = 1;
    drr->gradientType = "analytic";
    drr->hessianType  = "none";
  }
  env.insert_nodes(dme, dmo, dv, di, dr);

  if( !finalize )
    return p_env;

  // once done with changes: check database, broadcast, and construct iterators
  env.done_modifying_db();

  // plug the client's interface (function evaluator) into the Dakota environment
  serial_interface_plugin(env);

  return p_env;
}

#endif
