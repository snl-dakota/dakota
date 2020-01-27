/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_tabular_io.hpp"
#include "opt_tpl_rol_test_interface.hpp"
#include "opt_tpl_test.hpp"
#include "opt_tpl_test_fixture.hpp"  // for plugin interface
#include <string>
#include <Teuchos_UnitTestHarness.hpp> 

TEUCHOS_UNIT_TEST(surrogates_gp,base_test)
{
  // Dakota input string:
  static const char dakota_input[] =
    "environment \n"
    "  method_pointer 'EvalSurrogate' \n"
    "  tabular_data \n"
    "    tabular_data_file 'dak_surrogates_gp_evals.dat' \n"
    "      freeform \n"
    "method \n"
    "  id_method 'EvalSurrogate' \n"
    "  model_pointer 'SurrogateModel' \n"
    "  list_parameter_study \n"
    "    import_points_file 'gauss_proc_test_files/gauss_proc_eval_points.dat' \n"
    "      annotated \n"
    "model \n"
    "  id_model 'SurrogateModel' \n"
    "  surrogate \n"
    "    global \n"
    "      actual_model_pointer 'SimulationModel' \n"
    "      gauss_proc \n"
    "      import_points_file 'gauss_proc_test_files/gauss_proc_build_points.dat' \n"
    "        annotated \n"
    "variables \n"
    "  id_variables 'vars' \n"
    "  uniform_uncertain 2 \n"
    "    lower_bounds -2.0 -2.0 \n"
    "    upper_bounds  2.0  2.0 \n"
    "    descriptors  'x1' 'x2' \n"
    "responses \n"
    "  id_responses 'resps' \n"
    "  response_functions 1 \n"
    "    descriptors 'herbie' \n"
    "  no_gradients \n"
    "  no_hessians \n"
    "model \n"
    "  id_model = 'SimulationModel' \n"
    "  single \n"
    "  variables_pointer 'vars' \n"
    "  responses_pointer 'resps' \n";

  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Opt_TPL_Test::create_env(dakota_input));
  Dakota::LibraryEnvironment & env = *p_env;
  Dakota::ProblemDescDB& problem_db = env.problem_description_db();  

  // Execute the environment
  env.execute();

  // Read in the tabular output file
  const std::string tabular_data_name = "dak_surrogates_gp_evals.dat";
  const int NUM_COLS = 3;
  const int NUM_ROWS = 4;

  RealMatrix tabular_data;
  Dakota::TabularIO::read_data_tabular(tabular_data_name, "", tabular_data, NUM_ROWS, NUM_COLS, TABULAR_NONE, true);

  // Verify equality of gold data
  double gold_values[NUM_COLS][NUM_ROWS] =
    {{ 0.2, -0.3, 0.4, -0.25 },  //x1
     { 0.45, -0.7, -0.1, 0.33 }, //x2
     { 0.7798594591, 0.8467118256, 0.7445028879, 0.7465409943 }}; //herbie

  for(int i = 0; i < NUM_COLS; i++) {
    for(int j = 0; j < NUM_ROWS; j++) {
      TEST_FLOATING_EQUALITY( tabular_data[i][j], gold_values[i][j], 1.e-14 );
    }
  }
}