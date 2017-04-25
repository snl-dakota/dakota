
#include "APPSOptimizer.hpp"

#include "opt_tpl_test.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;

// //----------------------------------------------------------------

// TEUCHOS_UNIT_TEST(opt_test_cyl_head_coliny, hopspack)
// {
//   /// Default Dakota input string for serial case (cyl_head):
//   static const char serial_input[] = 
//     " method,"
//     "   max_iterations = 20000"
//     "   max_function_evaluations 20000"
//     "   coliny_ea"
//     "     seed = 11011011"
//     "     population_size = 100"
//     "     fitness_type merit_function"
//     "     mutation_type offset_normal"
//     "     mutation_rate 1.0"
//     "     crossover_type two_point"
//     "     crossover_rate 0.0"
//     "     replacement_type chc = 10"
//     " variables,"
//     "   continuous_design = 2"
//     "     initial_point    2.12         1.76"
//     "     upper_bounds     2.164        4.0"
//     "     lower_bounds     1.5          0.0"
//     "     descriptors      'intake_dia' 'flatness'"
//     " interface,"
//     "   direct"
//     "     analysis_driver = 'cyl_head'"
//     " responses,"
//     "   num_objective_functions = 1"
//     "   nonlinear_inequality_constraints = 3"
//     "   no_gradients"
//     "   no_hessians";

//   Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(serial_input);
//   Dakota::LibraryEnvironment & env = *p_env;

//   if (env.parallel_library().mpirun_flag())
//     TEST_ASSERT( false ); // This test only works for serial builds

//   // Execute the environment
//   env.execute();

//   const Real max_tol = 1.e-5;

//   // retrieve the final parameter values
//   const Variables& vars = env.variables_results();

//   TEST_FLOATING_EQUALITY( vars.continuous_variable(0), 2.1224215765, max_tol );
//   TEST_FLOATING_EQUALITY( vars.continuous_variable(1), 1.7659069377, max_tol );

//   // retrieve the final response values
//   const Response& resp  = env.response_results();
//   //resp.function_values().print(Cout);
//   //resp.write(Cout);
//   TEST_FLOATING_EQUALITY( resp.function_value(0), -2.4614299775, max_tol );
// }

// //----------------------------------------------------------------

// TEUCHOS_UNIT_TEST(opt_test_herbie_coliny, hopspack)
// {
//   /// Default Dakota input string for serial case (cyl_head):
//   static const char serial_input[] = 
//     " method,"
//     "   max_iterations = 20000"
//     "   max_function_evaluations 20000"
//     "   coliny_ea"
//     "     seed = 11011011"
//     "     population_size = 100"
//     "     fitness_type merit_function"
//     "     mutation_type offset_normal"
//     "     mutation_rate 1.0"
//     "     crossover_type two_point"
//     "     crossover_rate 0.0"
//     "     replacement_type chc = 10"
//     " variables,"
//     "   continuous_design = 2"
//     "     initial_point  -0.9 -1.1"
//     "     upper_bounds 2.0  2.0"
//     "     lower_bounds -2.0 -2.0"
//     "     descriptors 'x1'  'x2'"
//     " interface,"
//     "   direct"
//     "     analysis_driver = 'herbie'"
//     " responses,"
//     "   num_objective_functions = 1"
//     "   no_gradients"
//     "   no_hessians";

//   Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(serial_input);
//   Dakota::LibraryEnvironment & env = *p_env;

//   if (env.parallel_library().mpirun_flag())
//     TEST_ASSERT( false ); // This test only works for serial builds

//   // Execute the environment
//   env.execute();

//   const Real max_tol = 1.e-5;

//   // retrieve the final parameter values
//   const Variables& vars = env.variables_results();

//   TEST_FLOATING_EQUALITY( vars.continuous_variable(0), -1.04082591, max_tol );
//   TEST_FLOATING_EQUALITY( vars.continuous_variable(1),  -1.04082591, max_tol );

//   // retrieve the final response values
//   const Response& resp  = env.response_results();
//   //resp.function_values().print(Cout);
//   //resp.write(Cout);
//   TEST_FLOATING_EQUALITY( resp.function_value(0), -1.1268717458, max_tol );
// }

// //----------------------------------------------------------------

// TEUCHOS_UNIT_TEST(opt_test_smooth_herbie_coliny, hopspack)
// {
//   /// Default Dakota input string for serial case (cyl_head):
//   static const char serial_input[] = 
//     " method,"
//     "   max_iterations = 20000"
//     "   max_function_evaluations 20000"
//     "   coliny_ea"
//     "     seed = 11011011"
//     "     population_size = 100"
//     "     fitness_type merit_function"
//     "     mutation_type offset_normal"
//     "     mutation_rate 1.0"
//     "     crossover_type two_point"
//     "     crossover_rate 0.0"
//     "     replacement_type chc = 10"
//     " variables,"
//     "   continuous_design = 2"
//     "     initial_point  -0.9 -1.1"
//     "     upper_bounds 2.0  2.0"
//     "     lower_bounds -2.0 -2.0"
//     "     descriptors 'x1'  'x2'"
//     " interface,"
//     "   direct"
//     "     analysis_driver = 'smooth_herbie'"
//     " responses,"
//     "   num_objective_functions = 1"
//     "   no_gradients"
//     "   no_hessians";

//   Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(serial_input);
//   Dakota::LibraryEnvironment & env = *p_env;

//   if (env.parallel_library().mpirun_flag())
//     TEST_ASSERT( false ); // This test only works for serial builds

//   // Execute the environment
//   env.execute();

//   const Real max_tol = 1.e-5;

//   // retrieve the final parameter values
//   const Variables& vars = env.variables_results();

//   TEST_FLOATING_EQUALITY( vars.continuous_variable(0), -1.04082591, max_tol );
//   TEST_FLOATING_EQUALITY( vars.continuous_variable(1),  -1.04082591, max_tol );

//   // retrieve the final response values
//   const Response& resp  = env.response_results();
//   //resp.function_values().print(Cout);
//   //resp.write(Cout);
//   TEST_FLOATING_EQUALITY( resp.function_value(0), -1.1268717458, max_tol );
// }

// //----------------------------------------------------------------

// TEUCHOS_UNIT_TEST(opt_test_text_book_disc_range_coliny, hopspack)
// {
//   /// Default Dakota input string for serial case (cyl_head):
//   static const char serial_input[] = 
//     " method,"
//     "   max_iterations = 1000"
//     "   max_function_evaluations 1000"
//     "   coliny_ea"
//     "     seed = 11011011"
//     "     population_size = 50"
//     "     fitness_type merit_function"
//     "     mutation_type offset_normal"
//     "     mutation_rate 1.0"
//     "     crossover_type two_point"
//     "     crossover_rate 0.0"
//     "     replacement_type chc = 10"
//     " variables,"
//     "   discrete_design_range = 3"
//     "     initial_point      -2     2     -3"
//     "     lower_bounds       -4     -9     -9"
//     "     upper_bounds       4     9     9"
//     "     descriptors      'y1'   'y2'   'y3'"
//     " interface,"
//     "   direct"
//     "     analysis_driver = 'text_book'"
//     " responses,"
//     "   num_objective_functions = 1"
//     "   no_gradients"
//     "   no_hessians";

//   Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(serial_input);
//   Dakota::LibraryEnvironment & env = *p_env;

//   if (env.parallel_library().mpirun_flag())
//     TEST_ASSERT( false ); // This test only works for serial builds

//   // Execute the environment
//   env.execute();

//   const Real max_tol = 1.e-5;

//   // retrieve the final parameter values
//   const Variables& vars = env.variables_results();

//   TEST_FLOATING_EQUALITY( vars.continuous_variable(0), -1.04082591, max_tol );
//   TEST_FLOATING_EQUALITY( vars.continuous_variable(1),  -1.04082591, max_tol );

//   // retrieve the final response values
//   const Response& resp  = env.response_results();
//   //resp.function_values().print(Cout);
//   //resp.write(Cout);
//   TEST_FLOATING_EQUALITY( resp.function_value(0), -1.1268717458, max_tol );
// }

// //----------------------------------------------------------------

// TEUCHOS_UNIT_TEST(opt_test_text_book_disc_set_coliny, hopspack)
// {
//   /// Default Dakota input string for serial case (cyl_head):
//   static const char serial_input[] = 
//     " method,"
//     "   max_iterations = 1000"
//     "   max_function_evaluations 1000"
//     "   coliny_ea"
//     "     seed = 11011011"
//     "     population_size = 50"
//     "     fitness_type merit_function"
//     "     mutation_type offset_normal"
//     "     mutation_rate 1.0"
//     "     crossover_type two_point"
//     "     crossover_rate 0.0"
//     "     replacement_type chc = 10"
//     " variables,"
//     "     discrete_design_set"
//     "     integer = 3"
//     "     initial_point     -2   -4  2"
//     "     num_set_values =  5   5   5"
//     "     set_values = -4 -2 0 2 4 -4 -2 0 2 4 -4 -2 1 2 4"
//     "     descriptors      'y1'   'y2'   'y3'"
//     " interface,"
//     "   direct"
//     "     analysis_driver = 'text_book'"
//     " responses,"
//     "   num_objective_functions = 1"
//     "   no_gradients"
//     "   no_hessians";

//   Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(serial_input);
//   Dakota::LibraryEnvironment & env = *p_env;

//   if (env.parallel_library().mpirun_flag())
//     TEST_ASSERT( false ); // This test only works for serial builds

//   // Execute the environment
//   env.execute();

//   const Real max_tol = 1.e-5;

//   // retrieve the final parameter values
//   const Variables& vars = env.variables_results();

//   TEST_FLOATING_EQUALITY( vars.continuous_variable(0), -1.04082591, max_tol );
//   TEST_FLOATING_EQUALITY( vars.continuous_variable(1),  -1.04082591, max_tol );

//   // retrieve the final response values
//   const Response& resp  = env.response_results();
//   //resp.function_values().print(Cout);
//   //resp.write(Cout);
//   TEST_FLOATING_EQUALITY( resp.function_value(0), -1.1268717458, max_tol );
// }

// //----------------------------------------------------------------

// TEUCHOS_UNIT_TEST(opt_test_text_book_multi_coliny, hopspack)
// {
//   /// Default Dakota input string for serial case (cyl_head):
//   static const char serial_input[] = 
//     " method,"
//     "   max_function_evaluations 2000"
//     "   moga"
//     "     convergence_type metric_tracker"
//     "     percent_change = 1.0e-10 num_generations = 1000"
//     "     seed = 10983"
//     "     crossover_type shuffle_random"
//     "     num_offspring = 2 num_parents = 2"
//     "     mutation_type replace_uniform"
//     "     mutation_rate = 0.1"
//     "     fitness_type domination_count"
//     "     replacement_type below_limit = 6"
//     "     shrinkage_fraction = 0.9"
//     " variables,"
//     "     continuous_design = 2"
//     "     initial_point    0.5       0.5"
//     "     upper_bounds     1         1"
//     "     lower_bounds     0         0"
//     "     descriptors      'x1'    'x2'"
//     " interface,"
//     "   direct"
//     "     analysis_driver = 'mogatest3'"
//     " responses,"
//     "   num_objective_functions = 2"
//     "   nonlinear_inequality_constraints = 2"
//     "   upper_bounds = 0.0 0.0"
//     "   no_gradients"
//     "   no_hessians";

//   Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(serial_input);
//   Dakota::LibraryEnvironment & env = *p_env;

//   if (env.parallel_library().mpirun_flag())
//     TEST_ASSERT( false ); // This test only works for serial builds

//   // Execute the environment
//   env.execute();

//   const Real max_tol = 1.e-2;

//   // retrieve the final parameter values
//   const Variables& vars = env.variables_results();

//   TEST_FLOATING_EQUALITY( vars.continuous_variable(0)+1.0, 1.0, max_tol );
//   TEST_FLOATING_EQUALITY( vars.continuous_variable(1),  1.0, max_tol );

//   // retrieve the final response values
//   const Response& resp  = env.response_results();
//   //resp.function_values().print(Cout);
//   //resp.write(Cout);
//   TEST_FLOATING_EQUALITY( resp.function_value(0), 6.0, max_tol );
//   TEST_FLOATING_EQUALITY( resp.function_value(1)+1.0, 1.0, max_tol );
// }

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_test_text_book_multi_disc_set_coliny, hopspack)
{
  /// Default Dakota input string for serial case (cyl_head):
  static const char serial_input[] = 
    " method,"
    "   max_function_evaluations 10000"
    "   moga"
    "     convergence_type metric_tracker"
    "     percent_change = 1.0e-10 num_generations = 100"
    "     seed = 10983"
    "     crossover_type shuffle_random"
    "     num_offspring = 2 num_parents = 2"
    "     mutation_type replace_uniform"
    "     mutation_rate = 0.1"
    "     fitness_type domination_count"
    "     replacement_type below_limit = 6"
    "     shrinkage_fraction = 0.9"
    " variables,"
    " discrete_design_set"
    "   integer = 3"
    "   initial_point     0    0  0"
    "   num_set_values = 5 5 5"
    "   set_values = -4 -2 0 2 4 -4 -2 0 2 4 -4 -2 0 2 4"
    " interface,"
    "   direct"
    "     analysis_driver = 'mogatest1'"
    " responses,"
    "   num_objective_functions = 2"
    "   no_gradients"
    "   no_hessians";

  Dakota::LibraryEnvironment * p_env = Opt_TPL_Test::create_env(serial_input);
  Dakota::LibraryEnvironment & env = *p_env;

  if (env.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  // Execute the environment
  env.execute();

  const Real max_tol = 1.e-2;

  // retrieve the final parameter values
  const Variables& vars = env.variables_results();

  TEST_FLOATING_EQUALITY( ((double)vars.discrete_int_variable(0))+1.0, 1.0, max_tol );
  TEST_FLOATING_EQUALITY( ((double)vars.discrete_int_variable(1))+1.0,  1.0, max_tol );
  TEST_FLOATING_EQUALITY( ((double)vars.discrete_int_variable(2))+1.0,  1.0, max_tol );

  // retrieve the final response values
  const Response& resp  = env.response_results();
  //resp.function_values().print(Cout);
  //resp.write(Cout);
  TEST_FLOATING_EQUALITY( resp.function_value(0), 6.3212055883e-01, max_tol );
  TEST_FLOATING_EQUALITY( resp.function_value(1), 6.3212055883e-01, max_tol );
}