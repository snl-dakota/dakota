/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifdef HAVE_DEMO_TPL

#include <gtest/gtest.h>

#include <string>
#include <map>

#include "opt_tpl_test.hpp"

#include "DemoOptimizer.hpp"
#include "DakotaTPLDataTransfer.hpp"
#include "DakotaIterator.hpp"

using namespace Dakota;

namespace {
  /// Dakota input string:
  string baseline_text_book_input =
   "environment                          "
   "    write_restart 'opt_tpl_adapters.rst' "
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


  std::shared_ptr<Dakota::Optimizer> 
    get_optimizer(std::shared_ptr<Dakota::LibraryEnvironment> p_env, bool verbose = false)
    {
      Dakota::ProblemDescDB & problem_db = p_env->problem_description_db();
      Dakota::IteratorList & iter_list = Dakota::Iterator::iterator_cache(problem_db);
      std::shared_ptr<Iterator> dak_iter = *iter_list.begin();
      if( verbose )
      {
        Cout << "The iterator is a : " << dak_iter->method_string() << endl;
        dak_iter->print_results(Cout);
      }
      std::shared_ptr<Dakota::Optimizer> optimizer = std::static_pointer_cast<Dakota::Optimizer>(dak_iter);

      return optimizer;
    }
}

//----------------------------------------------------------------

TEST(opt_tpl_adapters_tests, test_nln_ineq_demo)
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
      FAIL(); // This test only works for serial builds
    // Execute the environment
    p_env->execute();
  }

  std::shared_ptr<Dakota::DemoTPLOptimizer> demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));

  std::shared_ptr<TPLDataTransfer> data_xfer = demo_optimizer->get_data_transfer_helper();
  DemoOptTraits::VecT nln_ineqs(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_ineqs[0]), 1.e-12/100. );


  //*********************************

  // Now test adapters when using nonlinear ineq upper bounds

  text_book_input +=
   "    nonlinear_inequality_upper_bounds =  5.0  ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));
  data_xfer = demo_optimizer->get_data_transfer_helper();
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -5.5 / nln_ineqs[0]), 1.e-12/100. );

  // *********************************

  // Now test adapters when using nonlinear ineq lower bounds

  text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_inequality_constraints = 1      "
   "    nonlinear_inequality_lower_bounds = -2.0  "
   "    descriptors 'f' 'c1'                      ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));
  data_xfer = demo_optimizer->get_data_transfer_helper();
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((2 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_ineqs[0]), 1.e-12/100. );
  EXPECT_LT(std::fabs(1. - -1.5 / nln_ineqs[1]), 1.e-12/100. );
  
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

  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));
  data_xfer = demo_optimizer->get_data_transfer_helper();
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((2 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -5.5 / nln_ineqs[0]), 1.e-12/100. );
  EXPECT_LT(std::fabs(1. - -1.5 / nln_ineqs[1]), 1.e-12/100. );
}

//----------------------------------------------------------------

TEST(opt_tpl_adapters_tests, test_nln_eq_demo)
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
      FAIL(); // This test only works for serial builds
    // Execute the environment
    p_env->execute();
  }

  std::shared_ptr<Dakota::DemoTPLOptimizer> demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));
  
  std::shared_ptr<TPLDataTransfer> data_xfer = demo_optimizer->get_data_transfer_helper();
  DemoOptTraits::VecT nln_eqs(data_xfer->num_tpl_nonlin_eq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_eqs[0]), 1.e-12/100. );


  // Now test adapters when using nonzero nonlinear equality targets
  text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_equality_constraints = 1      "
   "    nonlinear_equality_targets = -.5        "
   "    descriptors 'f' 'c1'                    ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));
  data_xfer = demo_optimizer->get_data_transfer_helper();
  nln_eqs.resize(data_xfer->num_tpl_nonlin_eq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_eqs[0]), 1.e-12/100. );
}

//----------------------------------------------------------------

TEST(opt_tpl_adapters_tests, test_nln_mixed_demo)
{
  /// Dakota input string:
  string text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_equality_constraints = 1      "
   "    nonlinear_inequality_constraints = 1    "
   "    descriptors 'f' 'c1' 'c2'               ";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input.c_str()));

  // boilerplate to create Dakota objects, etc...
  {
    if (p_env->parallel_library().mpirun_flag())
      FAIL(); // This test only works for serial builds
    // Execute the environment
    p_env->execute();
  }

  std::shared_ptr<Dakota::DemoTPLOptimizer> demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));

  std::shared_ptr<TPLDataTransfer> data_xfer = demo_optimizer->get_data_transfer_helper();
  DemoOptTraits::VecT nln_eqs(data_xfer->num_tpl_nonlin_eq_constraints());
  DemoOptTraits::VecT nln_ineqs(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs);
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - 80.0 / nln_ineqs[0]), 1.e-12/100. );


  //*********************************

  // Now test adapters when using nonzero nonlinear equality targets and nonlinear ineq upper bounds

  text_book_input +=
   "    nonlinear_equality_targets = -.5          "
   "    nonlinear_inequality_upper_bounds =  5.0  ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));
  data_xfer = demo_optimizer->get_data_transfer_helper();
  nln_eqs.resize(data_xfer->num_tpl_nonlin_eq_constraints());
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs);
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - 75.0 / nln_ineqs[0]), 1.e-12/100. );

  // *********************************

  // Now test mixed adapters when using nonlinear ineq lower bounds

  text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_equality_constraints = 1      "
   "    nonlinear_inequality_constraints = 1      "
   "    nonlinear_inequality_lower_bounds = -2.0  ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));
  data_xfer = demo_optimizer->get_data_transfer_helper();
  nln_eqs.resize(data_xfer->num_tpl_nonlin_eq_constraints());
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs);
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((2 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. -  80.0 / nln_ineqs[0]), 1.e-12/100. );
  EXPECT_LT(std::fabs(1. - -82.0 / nln_ineqs[1]), 1.e-12/100. );
  
  // *********************************

  // Now test adapters when using nonzero nonlinear equality targets and inequality lower and upper bounds

  text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_equality_constraints = 1        "
   "    nonlinear_equality_targets = -.5          "
   "    nonlinear_inequality_constraints = 1      "
   "    nonlinear_inequality_lower_bounds = -2.0  "
   "    nonlinear_inequality_upper_bounds =  5.0  ";
   "    descriptors 'f' 'c1'                      ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));
  data_xfer = demo_optimizer->get_data_transfer_helper();
  nln_eqs.resize(data_xfer->num_tpl_nonlin_eq_constraints());
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs);
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((2 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. -  75.0 / nln_ineqs[0]), 1.e-12/100. );
  EXPECT_LT(std::fabs(1. - -82.0 / nln_ineqs[1]), 1.e-12/100. );
}


//----------------------------------------------------------------
//  Define an alternate traits class for purposes of testing 
//  the various combinations of constraint formats
//----------------------------------------------------------------

namespace 
{
  template< NONLINEAR_INEQUALITY_FORMAT NLINEQ_F,
            NONLINEAR_EQUALITY_FORMAT   NLEQ_F   >
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

      /// Return format for nonlinear inequality constraints
      NONLINEAR_EQUALITY_FORMAT nonlinear_equality_format() override
      { return NLEQ_F; }

  }; // mock traits for unit testing
}

//----------------------------------------------------------------

TEST(opt_tpl_adapters_tests, test_nln_ineq_traits)
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
      FAIL(); // This test only works for serial builds
    // Execute the environment
    p_env->execute();
  }

  // The Demo optimizer is only needed to get the model and responses
  std::shared_ptr<Dakota::DemoTPLOptimizer> demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));

  using TRAITS_TYPE1 = UnitTestTraits<NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_LOWER, NONLINEAR_EQUALITY_FORMAT::TRUE_EQUALITY>;
  using TRAITS_TYPE2 = UnitTestTraits<NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED,       NONLINEAR_EQUALITY_FORMAT::TRUE_EQUALITY>;

  std::shared_ptr<TraitsBase> test_traits1(new TRAITS_TYPE1()); 
  std::shared_ptr<TraitsBase> test_traits2(new TRAITS_TYPE2()); 

  //*********************************

  // Test adapters when using default (no specified) nonlinear ineq bounds

  // ONE_SIDED_LOWER
  std::shared_ptr<TPLDataTransfer> data_xfer(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, *demo_optimizer->iterated_model() );

  TRAITS_TYPE1::VecT nln_ineqs(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - 0.5 / nln_ineqs[0]), 1.e-12/100. );

  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, *demo_optimizer->iterated_model() );

  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_ineqs[0]), 1.e-12/100. );


  //*********************************

  // Now test adapters when using nonlinear ineq upper bounds

  text_book_input +=
   "    nonlinear_inequality_upper_bounds =  5.0  ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  // The Demo optimizer is only needed to get the model and responses
  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));

  // ONE_SIDED_LOWER
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, *demo_optimizer->iterated_model() );

  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - 5.5 / nln_ineqs[0]), 1.e-12/100. );

  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, *demo_optimizer->iterated_model() );

  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_ineqs[0]), 1.e-12/100. );


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
  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));

  // ONE_SIDED_LOWER
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, *demo_optimizer->iterated_model() );

  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((2 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - 0.5 / nln_ineqs[0]), 1.e-12/100. );
  EXPECT_LT(std::fabs(1. - 1.5 / nln_ineqs[1]), 1.e-12/100. );
  
  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, *demo_optimizer->iterated_model() );

  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_ineqs[0]), 1.e-12/100. );


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
  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));

  // ONE_SIDED_LOWER
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, *demo_optimizer->iterated_model() );

  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((2 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - 5.5 / nln_ineqs[0]), 1.e-12/100. );
  EXPECT_LT(std::fabs(1. - 1.5 / nln_ineqs[1]), 1.e-12/100. );

  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, *demo_optimizer->iterated_model() );

  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_ineqs[0]), 1.e-12/100. );
}

//----------------------------------------------------------------

TEST(opt_tpl_adapters_tests, test_nln_mixed_traits)
{
  /// Dakota input string:
  string text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_equality_constraints = 1        "
   "    nonlinear_inequality_constraints = 1      "
   "    descriptors 'f' 'c1' 'c2'                 ";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(text_book_input.c_str()));

  // boilerplate to create Dakota objects, etc...
  {
    if (p_env->parallel_library().mpirun_flag())
      FAIL(); // This test only works for serial builds
    // Execute the environment
    p_env->execute();
  }

  // The Demo optimizer is only needed to get the model and responses
  std::shared_ptr<Dakota::DemoTPLOptimizer> demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));

  using TRAITS_TYPE1 = UnitTestTraits<NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_LOWER, NONLINEAR_EQUALITY_FORMAT::TRUE_EQUALITY>;
  using TRAITS_TYPE2 = UnitTestTraits<NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED,       NONLINEAR_EQUALITY_FORMAT::TRUE_EQUALITY>;

  std::shared_ptr<TraitsBase> test_traits1(new TRAITS_TYPE1()); 
  std::shared_ptr<TraitsBase> test_traits2(new TRAITS_TYPE2()); 

  //*********************************

  // Test adapters when using default (no specified) nonlinear ineq bounds

  // ONE_SIDED_LOWER
  std::shared_ptr<TPLDataTransfer> data_xfer(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, *demo_optimizer->iterated_model() );

  TRAITS_TYPE1::VecT nln_eqs  (data_xfer->num_tpl_nonlin_eq_constraints());
  TRAITS_TYPE1::VecT nln_ineqs(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota  (demo_optimizer->iterated_model()->current_response(), nln_eqs  );
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -80.0 / nln_ineqs[0]), 1.e-12/100. );

  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, *demo_optimizer->iterated_model() );

  nln_eqs.resize  (data_xfer->num_tpl_nonlin_eq_constraints());
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota  (demo_optimizer->iterated_model()->current_response(), nln_eqs  );
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - 80.0 / nln_ineqs[0]), 1.e-12/100. );


  //*********************************

  // Now test adapters when using nonlinear ineq upper bounds

  text_book_input +=
   "    nonlinear_equality_targets = -.5          "
   "    nonlinear_inequality_upper_bounds =  5.0  ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  // The Demo optimizer is only needed to get the model and responses
  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));

  // ONE_SIDED_LOWER
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, *demo_optimizer->iterated_model() );

  nln_eqs.resize  (data_xfer->num_tpl_nonlin_eq_constraints());
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota  (demo_optimizer->iterated_model()->current_response(), nln_eqs);
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -75.0 / nln_ineqs[0]), 1.e-12/100. );

  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, *demo_optimizer->iterated_model() );

  nln_eqs.resize  (data_xfer->num_tpl_nonlin_eq_constraints());
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota  (demo_optimizer->iterated_model()->current_response(), nln_eqs);
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - 80.0 / nln_ineqs[0]), 1.e-12/100. );


  // *********************************

  // Now test adapters when using nonlinear ineq lower bounds

  text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_equality_constraints = 1        "
   "    nonlinear_inequality_constraints = 1      "
   "    nonlinear_inequality_lower_bounds = -2.0  "
   "    descriptors 'f' 'c1' 'c2'                 ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  // The Demo optimizer is only needed to get the model and responses
  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));

  // ONE_SIDED_LOWER
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, *demo_optimizer->iterated_model() );

  nln_eqs.resize  (data_xfer->num_tpl_nonlin_eq_constraints());
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota  (demo_optimizer->iterated_model()->current_response(), nln_eqs);
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((2 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -80.0 / nln_ineqs[0]), 1.e-12/100. );
  EXPECT_LT(std::fabs(1. -  82.0 / nln_ineqs[1]), 1.e-12/100. );
  
  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, *demo_optimizer->iterated_model() );

  nln_eqs.resize  (data_xfer->num_tpl_nonlin_ineq_constraints());
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota  (demo_optimizer->iterated_model()->current_response(), nln_eqs);
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(1. - -0.5 / nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - 80.0 / nln_ineqs[0]), 1.e-12/100. );


  // *********************************

  // Now test adapters when using nonlinear ineq lower and upper bounds

  text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_equality_constraints = 1        "
   "    nonlinear_equality_targets = -.5          "
   "    nonlinear_inequality_constraints = 1      "
   "    nonlinear_inequality_lower_bounds = -2.0  "
   "    nonlinear_inequality_upper_bounds =  5.0  ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  // The Demo optimizer is only needed to get the model and responses
  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));

  // ONE_SIDED_LOWER
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits1, *demo_optimizer->iterated_model() );

  nln_eqs.resize  (data_xfer->num_tpl_nonlin_eq_constraints());
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota  (demo_optimizer->iterated_model()->current_response(), nln_eqs);
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((2 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - -75.0 / nln_ineqs[0]), 1.e-12/100. );
  EXPECT_LT(std::fabs(1. -  82.0 / nln_ineqs[1]), 1.e-12/100. );

  // TWO_SIDED
  data_xfer.reset(new TPLDataTransfer()); 
  data_xfer->configure_data_adapters( test_traits2, *demo_optimizer->iterated_model() );

  nln_eqs.resize  (data_xfer->num_tpl_nonlin_eq_constraints());
  nln_ineqs.resize(data_xfer->num_tpl_nonlin_ineq_constraints());
  data_xfer->get_nonlinear_eq_constraints_from_dakota  (demo_optimizer->iterated_model()->current_response(), nln_eqs);
  data_xfer->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs);

  EXPECT_TRUE((1 == nln_eqs.size()));
  EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_eqs[0]), 1.e-12/100. );
  EXPECT_TRUE((1 == nln_ineqs.size()));
  EXPECT_LT(std::fabs(1. - 80.0 / nln_ineqs[0]), 1.e-12/100. );
}


//----------------------------------------------------------------

TEST(opt_tpl_adapters_tests, test_nln_eq_traits)
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
      FAIL(); // This test only works for serial builds
    // Execute the environment
    p_env->execute();
  }

  // The Demo optimizer is only needed to get the model and responses
  std::shared_ptr<Dakota::DemoTPLOptimizer> demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));
  
  using TRAITS_TYPE1 = UnitTestTraits<NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_UPPER, NONLINEAR_EQUALITY_FORMAT::TWO_INEQUALITY>;
  using TRAITS_TYPE2 = UnitTestTraits<NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_LOWER, NONLINEAR_EQUALITY_FORMAT::TWO_INEQUALITY>;
  using TRAITS_TYPE3 = UnitTestTraits<NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED,       NONLINEAR_EQUALITY_FORMAT::TWO_INEQUALITY>;

  std::shared_ptr<TraitsBase> test_traits1(new TRAITS_TYPE1()); 
  std::shared_ptr<TraitsBase> test_traits2(new TRAITS_TYPE2()); 
  std::shared_ptr<TraitsBase> test_traits3(new TRAITS_TYPE3()); 

  std::shared_ptr<TPLDataTransfer> data_xfer1(new TPLDataTransfer()); 
  std::shared_ptr<TPLDataTransfer> data_xfer2(new TPLDataTransfer()); 
  std::shared_ptr<TPLDataTransfer> data_xfer3(new TPLDataTransfer()); 
  data_xfer1->configure_data_adapters( test_traits1, *demo_optimizer->iterated_model() );
  data_xfer2->configure_data_adapters( test_traits2, *demo_optimizer->iterated_model() );
  data_xfer3->configure_data_adapters( test_traits3, *demo_optimizer->iterated_model() );

  DemoOptTraits::VecT nln_eqs1(data_xfer1->num_tpl_nonlin_eq_constraints());
  DemoOptTraits::VecT nln_eqs2(data_xfer2->num_tpl_nonlin_eq_constraints());
  DemoOptTraits::VecT nln_eqs3(data_xfer3->num_tpl_nonlin_eq_constraints());
  data_xfer1->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs1);
  data_xfer2->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs2);
  data_xfer3->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs3);

  // There should be no equality values because they are treated as two-sided inequalities
  EXPECT_TRUE((0 == nln_eqs1.size()));
  EXPECT_TRUE((0 == nln_eqs2.size()));
  EXPECT_TRUE((0 == nln_eqs3.size()));

  DemoOptTraits::VecT nln_ineqs1(data_xfer1->num_tpl_nonlin_ineq_constraints());
  DemoOptTraits::VecT nln_ineqs2(data_xfer2->num_tpl_nonlin_ineq_constraints());
  DemoOptTraits::VecT nln_ineqs3(data_xfer3->num_tpl_nonlin_ineq_constraints());
  data_xfer1->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs1);
  data_xfer2->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs2);
  data_xfer3->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs3);

  // BUT there should be two inequality constraints for each model equality constraint
  EXPECT_TRUE((2 == nln_ineqs1.size()));
  EXPECT_TRUE((2 == nln_ineqs2.size()));
  EXPECT_TRUE((2 == nln_ineqs3.size()));
  // ... and they are all treated the same because they are transformed in a manner that works with any of the
  //     three inequality constraint formats
  EXPECT_LT(std::fabs(1. - -0.5 / nln_ineqs1[0]), 1.e-12/100. );   EXPECT_LT(std::fabs(1. -  0.5 / nln_ineqs1[1]), 1.e-12/100. );
  EXPECT_LT(std::fabs(1. - -0.5 / nln_ineqs2[0]), 1.e-12/100. );   EXPECT_LT(std::fabs(1. -  0.5 / nln_ineqs2[1]), 1.e-12/100. );
  EXPECT_LT(std::fabs(1. - -0.5 / nln_ineqs3[0]), 1.e-12/100. );   EXPECT_LT(std::fabs(1. -  0.5 / nln_ineqs3[1]), 1.e-12/100. );


  // Now test adapters when using nonzero nonlinear equality targets
  text_book_input = baseline_text_book_input;
  text_book_input +=
   "    nonlinear_equality_constraints = 1      "
   "    nonlinear_equality_targets = -.5        "
   "    descriptors 'f' 'c1'                    ";
  p_env.reset(Opt_TPL_Test::create_env(text_book_input.c_str()));
  p_env->execute();

  demo_optimizer = std::static_pointer_cast<Dakota::DemoTPLOptimizer>(get_optimizer(p_env));

  data_xfer1.reset(new TPLDataTransfer()); 
  data_xfer2.reset(new TPLDataTransfer()); 
  data_xfer3.reset(new TPLDataTransfer()); 
  data_xfer1->configure_data_adapters( test_traits1, *demo_optimizer->iterated_model() );
  data_xfer2->configure_data_adapters( test_traits2, *demo_optimizer->iterated_model() );
  data_xfer3->configure_data_adapters( test_traits3, *demo_optimizer->iterated_model() );

  // Check correctness of equality constraints (shouldn't be any)
  nln_eqs1.resize(data_xfer1->num_tpl_nonlin_eq_constraints());
  nln_eqs2.resize(data_xfer2->num_tpl_nonlin_eq_constraints());
  nln_eqs3.resize(data_xfer3->num_tpl_nonlin_eq_constraints());

  data_xfer1->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs1);
  data_xfer2->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs2);
  data_xfer3->get_nonlinear_eq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_eqs3);

  EXPECT_TRUE((0 == nln_eqs1.size()));
  EXPECT_TRUE((0 == nln_eqs2.size()));
  EXPECT_TRUE((0 == nln_eqs3.size()));

  // Check correctness of inequality constraints
  nln_ineqs1.resize(data_xfer1->num_tpl_nonlin_ineq_constraints());
  nln_ineqs2.resize(data_xfer2->num_tpl_nonlin_ineq_constraints());
  nln_ineqs3.resize(data_xfer3->num_tpl_nonlin_ineq_constraints());

  data_xfer1->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs1);
  data_xfer2->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs2);
  data_xfer3->get_nonlinear_ineq_constraints_from_dakota(demo_optimizer->iterated_model()->current_response(), nln_ineqs3);

  EXPECT_TRUE((2 == nln_ineqs1.size()));
  EXPECT_TRUE((2 == nln_ineqs2.size()));
  EXPECT_TRUE((2 == nln_ineqs3.size()));
  EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_ineqs1[0]), 1.e-12/100. );   EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_ineqs1[1]), 1.e-12/100. );
  EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_ineqs2[0]), 1.e-12/100. );   EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_ineqs2[1]), 1.e-12/100. );
  EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_ineqs3[0]), 1.e-12/100. );   EXPECT_LT(std::fabs(/*1. - 0.0 / */ nln_ineqs3[1]), 1.e-12/100. );
}

//----------------------------------------------------------------

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#endif // HAVE_DEMO_TPL
