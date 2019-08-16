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

TEUCHOS_UNIT_TEST(opt_tpl_adapters, nln_ineq_demo)
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
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

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
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

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
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

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
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(2, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(-5.5, nln_ineqs[0], 1.e-12);
  TEST_FLOATING_EQUALITY(-1.5, nln_ineqs[1], 1.e-12);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_tpl_adapters, nln_eq_demo)
{
  /// Dakota input string:
  string text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_equality_constraints = 1      "
   "    descriptors 'f' 'c1'                    ";

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
  DemoOptTraits::VecT nln_eqs(data_xfer->num_nonlin_eq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_eqs);

  TEST_EQUALITY(1, nln_eqs.size());
  TEST_FLOATING_EQUALITY(-0.5, nln_eqs[0], 1.e-12);


  // Now test adapters when using nonzero nonlinear equality targets
  text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_equality_constraints = 1      "
   "    nonlinear_equality_targets = -.5        "
   "    descriptors 'f' 'c1'                    ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  demo_optimizer = dynamic_cast<Dakota::DemoTPLOptimizer*>(get_optimizer(p_env));
  data_xfer = demo_optimizer->get_data_transfer_helper();
  nln_eqs.resize(data_xfer->num_nonlin_eq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_eqs);

  TEST_EQUALITY(1, nln_eqs.size());
  TEST_FLOATING_EQUALITY(0.0, nln_eqs[0], 1.e-12);
}

//----------------------------------------------------------------
//  Define an alternate traits class for purposes of testing 
//  the various combinations of constraint formats
//----------------------------------------------------------------

namespace 
{
  template<NONLINEAR_INEQUALITY_FORMAT NLINEQ_F>
  class UnitTestTraits: public TraitsBase
  {
    public:

      //
      //- Heading: Constructor and destructor
      //

      /// Default constructor
      UnitTestTraits() { }

      /// Destructor
      virtual ~UnitTestTraits() { }

      /// default data type to be used by Dakota data adapters
      typedef std::vector<double> VecT;

      /// This is needed to handle constraints
      inline static double noValue()
      { return std::numeric_limits<Real>::max(); }

      //
      //- Heading: Virtual member function redefinitions
      //

      /// Return the flag indicating whether method supports continuous variables
      bool supports_continuous_variables() override
      { return true; }

      /// Return the flag indicating whether method supports nonlinear equality constrinats
      bool supports_nonlinear_equality() override
      { return true; }

      /// Return the flag indicating whether method supports nonlinear inequality constrinats
      bool supports_nonlinear_inequality() override
      { return true; }

      /// Return format for nonlinear inequality constraints
      NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format() override
      { return NLINEQ_F; }

  }; // mock traits for unit testing
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_tpl_adapters, nln_ineq_traits)
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

  // The Demo optimizer is only needed to get the model and responses
  Dakota::DemoTPLOptimizer * demo_optimizer = dynamic_cast<Dakota::DemoTPLOptimizer*>(get_optimizer(p_env));

  using TRAITS_TYPE1 = UnitTestTraits<NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_LOWER>;
  using TRAITS_TYPE2 = UnitTestTraits<NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED>;

  std::shared_ptr<TraitsBase> test_traits1(new TRAITS_TYPE1()); 
  std::shared_ptr<TraitsBase> test_traits2(new TRAITS_TYPE2()); 

  //*********************************

  // Test adapters when using default (no specified) nonlinear ineq bounds

  // ONE_SIDED_LOWER
  std::shared_ptr<TPLDataTransfer> data_xfer(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, demo_optimizer->iterated_model().user_defined_constraints() );

  TRAITS_TYPE1::VecT nln_ineqs(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(1, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(0.5, nln_ineqs[0], 1.e-12);

  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, demo_optimizer->iterated_model().user_defined_constraints() );

  nln_ineqs.resize(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(1, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(-0.5, nln_ineqs[0], 1.e-12);


  //*********************************

  // Now test adapters when using nonlinear ineq upper bounds

  text_book_input +=
   "    nonlinear_inequality_upper_bounds =  5.0  ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  // The Demo optimizer is only needed to get the model and responses
  demo_optimizer = dynamic_cast<Dakota::DemoTPLOptimizer*>(get_optimizer(p_env));

  // ONE_SIDED_LOWER
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, demo_optimizer->iterated_model().user_defined_constraints() );

  nln_ineqs.resize(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(1, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(5.5, nln_ineqs[0], 1.e-12);

  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, demo_optimizer->iterated_model().user_defined_constraints() );

  nln_ineqs.resize(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(1, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(-0.5, nln_ineqs[0], 1.e-12);


  // *********************************

  // Now test adapters when using nonlinear ineq lower bounds

  text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_inequality_constraints = 1      "
   "    nonlinear_inequality_lower_bounds = -2.0  "
   "    descriptors 'f' 'c1'                      ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  // The Demo optimizer is only needed to get the model and responses
  demo_optimizer = dynamic_cast<Dakota::DemoTPLOptimizer*>(get_optimizer(p_env));

  // ONE_SIDED_LOWER
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, demo_optimizer->iterated_model().user_defined_constraints() );

  nln_ineqs.resize(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(2, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(0.5, nln_ineqs[0], 1.e-12);
  TEST_FLOATING_EQUALITY(1.5, nln_ineqs[1], 1.e-12);
  
  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, demo_optimizer->iterated_model().user_defined_constraints() );

  nln_ineqs.resize(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(1, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(-0.5, nln_ineqs[0], 1.e-12);


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

  // The Demo optimizer is only needed to get the model and responses
  demo_optimizer = dynamic_cast<Dakota::DemoTPLOptimizer*>(get_optimizer(p_env));
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, demo_optimizer->iterated_model().user_defined_constraints() );

  nln_ineqs.resize(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(2, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(5.5, nln_ineqs[0], 1.e-12);
  TEST_FLOATING_EQUALITY(1.5, nln_ineqs[1], 1.e-12);

  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, demo_optimizer->iterated_model().user_defined_constraints() );

  nln_ineqs.resize(data_xfer->num_active_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model().current_response(), nln_ineqs);

  TEST_EQUALITY(1, nln_ineqs.size());
  TEST_FLOATING_EQUALITY(-0.5, nln_ineqs[0], 1.e-12);


}

//----------------------------------------------------------------
