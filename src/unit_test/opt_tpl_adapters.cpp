/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <string>
#include <map>
#include <Teuchos_UnitTestHarness.hpp>

#include "opt_tpl_test.hpp"

#include "DemoOptimizer.hpp"
#include "DakotaTPLDataTransfer.hpp"

using namespace Dakota;

namespace {
  /// Dakota input string:
  string baseline_text_book_input =
   "method,                              "
   "    demo_tpl                         "
   "    output silent                    "
   "    max_iterations = 0               "
   "    max_function_evaluations = 0     "
   "    solution_target = 0.05           "
   "                                     "
   "variables,                           "
   "    continuous_design = 2            "
   "    initial_point      2.0   9.0     "
   "    upper_bounds	   10.0   10.0   "
   "    lower_bounds      -10.0  -10.0   "
   "    descriptors	   'x1'  'x2'    "
   "                                     "
   "interface,                           "
   "    direct                           "
   "    analysis_driver = 'text_book'    "
   "                                     "
   "responses,                           "
   "    objective_functions = 1          "
   "    no_gradients                     "
   "    no_hessians                      ";


  Dakota::Optimizer * 
    get_optimizer(std::shared_ptr<Dakota::LibraryEnvironment> p_env, bool verbose = false)
    {
      Dakota::ProblemDescDB & problem_db = p_env->problem_description_db();
      Dakota::IteratorList & iter_list = problem_db.iterator_list();
      Dakota::Iterator & dak_iter = *iter_list.begin();
      if( verbose )
      {
        Cout << "The iterator is a : " << dak_iter.method_string() << endl;
        dak_iter.print_results(Cout);
      }
      Dakota::Optimizer * optimizer = dynamic_cast<Dakota::Optimizer*>(dak_iter.iterator_rep());

      return optimizer;
    }
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_tpl_adapters, nln_ineq)
{
  /// Dakota input string:
  string text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_inequality_constraints = 1      "
   "    descriptors 'f' 'c1'                      ";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input.c_str()));

  // boilerplate to create Dakota objects, etc...
  {
    if (p_env->parallel_library().mpirun_flag())
      TEST_ASSERT( false ); // This test only works for serial builds
    // Execute the environment
    p_env->execute();
  }

  Dakota::DemoTPLOptimizer * demo_optimizer = dynamic_cast<Dakota::DemoTPLOptimizer*>(get_optimizer(p_env));

  std::shared_ptr<TPLDataTransfer> data_xfer = demo_optimizer->get_data_transfer_helper();
  DemoOptTraits::VecT nln_ineqs(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(1, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(-0.5, nln_ineqs[0], 1.e-12);


  //*********************************

  // Now test adapters when using nonlinear ineq upper bounds

  text_book_input +=
   "    nonlinear_inequality_upper_bounds =  5.0  ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  demo_optimizer = dynamic_cast<Dakota::DemoTPLOptimizer*>(get_optimizer(p_env));
  data_xfer = demo_optimizer->get_data_transfer_helper();
  nln_ineqs.resize(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(1, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(-5.5, nln_ineqs[0], 1.e-12);

  // *********************************

  // Now test adapters when using nonlinear ineq lower bounds

  text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_inequality_constraints = 1      "
   "    nonlinear_inequality_lower_bounds = -2.0  "
   "    descriptors 'f' 'c1'                      ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  demo_optimizer = dynamic_cast<Dakota::DemoTPLOptimizer*>(get_optimizer(p_env));
  data_xfer = demo_optimizer->get_data_transfer_helper();
  nln_ineqs.resize(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(2, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(-0.5, nln_ineqs[0], 1.e-12);
  TEST_FLOATING_EQUALITY(-1.5, nln_ineqs[1], 1.e-12);
  
  // *********************************

  // Now test adapters when using nonlinear ineq lower and upper bounds
  text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_inequality_constraints = 1      "
   "    nonlinear_inequality_lower_bounds = -2.0  "
   "    nonlinear_inequality_upper_bounds =  5.0  ";
   "    descriptors 'f' 'c1'                      ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  demo_optimizer = dynamic_cast<Dakota::DemoTPLOptimizer*>(get_optimizer(p_env));
  data_xfer = demo_optimizer->get_data_transfer_helper();
  nln_ineqs.resize(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(2, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(-5.5, nln_ineqs[0], 1.e-12);
  TEST_FLOATING_EQUALITY(-1.5, nln_ineqs[1], 1.e-12);
}

//----------------------------------------------------------------
