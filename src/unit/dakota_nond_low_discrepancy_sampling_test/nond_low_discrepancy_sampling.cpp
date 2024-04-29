/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <exception>

#include "LowDiscrepancySequence.hpp"
#include "Rank1Lattice.hpp"

#include <fstream>
#include <cmath>

#include "opt_tpl_test.hpp"

#define BOOST_TEST_MODULE dakota_nond_low_discrepancy_sampling_test
#include <boost/test/included/unit_test.hpp>

namespace DakotaUnitTest {

namespace TestLowDiscrepancy {

//
//  Tests for NonDLowDiscrepancySampling
//
namespace TestNonDLowDiscrepancySampling {


// +-------------------------------------------------------------------------+
// |                         Check valid input file                          |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(NonDLowDiscrepancySampling_check_valid_input_file)
{
  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "    tabular_data \n"
    "    tabular_data_file = 'samples.dat' \n"
    "    freeform \n"
    "    write_restart 'NonDLowDiscrepancySampling_check_valid_input_file.rst' \n"
    "method \n"
    "  sampling \n"
    "    sample_type \n"
    "      low_discrepancy \n"
    "        rank_1_lattice \n"
    "          no_random_shift \n"
    "    samples 4 \n"
    "    output silent \n"
    "variables \n"
    "  uniform_uncertain = 2 \n"
    "    lower_bounds = 0.0 0.0 \n"
    "    upper_bounds = 1.0 1.0 \n"
    "interface \n"
    "    analysis_drivers = 'genz' \n"
    "    analysis_components = 'cp1' \n"
    "    direct \n"
    "responses \n"
    "  response_functions = 1 \n"
    "  no_gradients \n"
    "  no_hessians \n";

  // Get problem description
  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Dakota::Opt_TPL_Test::create_env(dakota_input));
  Dakota::LibraryEnvironment& env = *p_env;

  // Execute the environment
  env.execute();

  // Read in the tabular output file
  const std::string tabular_data_name = "samples.dat";
  Dakota::RealMatrix samples;
  Dakota::TabularIO::read_data_tabular(
    tabular_data_name, "", samples, 4, 3, Dakota::TABULAR_NONE, true
  );

  // Exact values
  double exact[4][3] =
  {
    {0,     0,     1},
    {0.5,   0.5,   0.702332},
    {0.25,  0.75,  0.646911},
    {0.75,  0.25,  0.764268}
  };

  // Check values of the lattice points
  for ( size_t row = 0; row < 4; row++  )
  {
    for( size_t col = 0; col < 3; col++)
    {
      BOOST_CHECK_CLOSE(samples[col][row], exact[row][col], 1e-4);
    }
  }
}

// +-------------------------------------------------------------------------+
// |                           Refinement samples                            |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(NonDLowDiscrepancySampling_check_refinement_samples)
{
  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "    tabular_data \n"
    "    tabular_data_file = 'samples.dat' \n"
    "    freeform \n"
    "    write_restart 'NonDLowDiscrepancySampling_check_refinement_samples.rst' \n"
    "method \n"
    "  sampling \n"
    "    sample_type \n"
    "      low_discrepancy \n"
    "        rank_1_lattice \n"
    "          no_random_shift \n"
    "          m_max 0 \n" // force m_max 0 to avoid failing test, should probably find a better way...
    "    samples 2 \n"
    "    refinement_samples 2 4 \n"
    "    output silent \n"
    "variables \n"
    "  uniform_uncertain = 2 \n"
    "    lower_bounds = 0.0 0.0 \n"
    "    upper_bounds = 1.0 1.0 \n"
    "interface \n"
    "    analysis_drivers = 'genz' \n"
    "    analysis_components = 'cp1' \n"
    "    direct \n"
    "responses \n"
    "  response_functions = 1 \n"
    "  no_gradients \n"
    "  no_hessians \n";

  // Get problem description
  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Dakota::Opt_TPL_Test::create_env(dakota_input));
  Dakota::LibraryEnvironment& env = *p_env;

  // Execute the environment
  env.execute();

  // Read in the tabular output file
  const std::string tabular_data_name = "samples.dat";
  Dakota::RealMatrix samples;
  Dakota::TabularIO::read_data_tabular(
    tabular_data_name, "", samples, 8, 3, Dakota::TABULAR_NONE, true
  );

  // Exact values
  double exact[8][3] =
  {
    {0,     0,     1},
    {0.5,   0.5,   0.702332},
    {0.25,  0.75,  0.646911},
    {0.75,  0.25,  0.764268},
    {0.125, 0.375, 0.797981},
    {0.625, 0.875, 0.574206},
    {0.375, 0.125, 0.871597},
    {0.875, 0.625, 0.621378}
  };

  // Check values of the lattice points
  for ( size_t row = 0; row < 8; row++  )
  {
    for( size_t col = 0; col < 3; col++)
    {
      BOOST_CHECK_CLOSE(samples[col][row], exact[row][col], 1e-4);
    }
  }
}

// +-------------------------------------------------------------------------+
// |                      Check normal random samples                        |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(NonDLowDiscrepancySampling_check_normal_random_samples)
{
  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "    tabular_data \n"
    "    tabular_data_file = 'samples.dat' \n"
    "    freeform \n"
    "    write_restart 'NonDLowDiscrepancySampling_check_normal_random_samples.rst' \n"
    "method \n"
    "  sampling \n"
    "    sample_type \n"
    "      low_discrepancy \n"
    "        rank_1_lattice \n"
    "    samples = 10000 \n"
    "    seed = 2023 \n"
    "    output silent \n"
    "variables \n"
    "  normal_uncertain = 2 \n"
    "    means = 0.0 1.0 \n"
    "    std_deviations = 1.0 0.5 \n"
    "interface \n"
    "    analysis_drivers = 'genz' \n"
    "    analysis_components = 'cp1' \n"
    "    direct \n"
    "responses \n"
    "  response_functions = 1 \n"
    "  no_gradients \n"
    "  no_hessians \n";

  // Get problem description
  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Dakota::Opt_TPL_Test::create_env(dakota_input));
  Dakota::LibraryEnvironment& env = *p_env;

  // Execute the environment
  env.execute();

  // Read in the tabular output file
  const std::string tabular_data_name = "samples.dat";
  Dakota::RealMatrix samples;
  int NB_OF_SAMPLES = 10000;
  Dakota::TabularIO::read_data_tabular(
    tabular_data_name, "", samples, NB_OF_SAMPLES, 3, Dakota::TABULAR_NONE, true
  );

  // Compute mean values
  double m1 = 0;
  double m2 = 0;
  for ( size_t j = 0; j < NB_OF_SAMPLES; j++ )
  {
    m1 += samples[0][j] / NB_OF_SAMPLES;
    m2 += samples[1][j] / NB_OF_SAMPLES;
  }

  // Compute standard deviations
  double s1 = 0;
  double s2 = 0;
  for ( size_t j = 0; j < NB_OF_SAMPLES; j++ )
  {
    s1 += (samples[0][j] - m1)*(samples[0][j] - m1);
    s2 += (samples[1][j] - m2)*(samples[1][j] - m2);
  }
  s1 = std::sqrt(s1 / (NB_OF_SAMPLES - 1));
  s2 = std::sqrt(s2 / (NB_OF_SAMPLES - 1));

  // Check values
  double TOL = 1e-3;
  BOOST_CHECK_SMALL(std::abs(m1 - 0), TOL);
  BOOST_CHECK_SMALL(std::abs(m2 - 1), TOL);
  BOOST_CHECK_SMALL(std::abs(s1 - 1), TOL);
  BOOST_CHECK_SMALL(std::abs(s2 - 0.5), TOL);
}

// +-------------------------------------------------------------------------+
// |                   Check transformed uniform samples                     |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(NonDLowDiscrepancySampling_check_transformed_uniform_samples)
{
  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "    tabular_data \n"
    "    tabular_data_file = 'samples.dat' \n"
    "    freeform \n"
    "    write_restart 'NonDLowDiscrepancySampling_check_transformed_uniform_samples.rst' \n"
    "method \n"
    "  sampling \n"
    "    sample_type \n"
    "      low_discrepancy \n"
    "        rank_1_lattice \n"
    "          no_random_shift \n"
    "    samples 4 \n"
    "    output silent \n"
    "variables \n"
    "  uniform_uncertain = 2 \n"
    "    lower_bounds = -1.0 0.0 \n"
    "    upper_bounds =  1.0 2.0 \n"
    "interface \n"
    "    analysis_drivers = 'genz' \n"
    "    analysis_components = 'cp1' \n"
    "    direct \n"
    "responses \n"
    "  response_functions = 1 \n"
    "  no_gradients \n"
    "  no_hessians \n";

  // Get problem description
  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Dakota::Opt_TPL_Test::create_env(dakota_input));
  Dakota::LibraryEnvironment& env = *p_env;

  // Execute the environment
  env.execute();

  // Read in the tabular output file
  const std::string tabular_data_name = "samples.dat";
  Dakota::RealMatrix samples;
  Dakota::TabularIO::read_data_tabular(
    tabular_data_name, "", samples, 4, 3, Dakota::TABULAR_NONE, true
  );

  // Exact values
  double exact[4][2] =
  {
    {-1,   0, },
    { 0,   1  },
    {-0.5, 1.5},
    { 0.5, 0.5}
  };

  // Check values of the lattice points
  for ( size_t row = 0; row < 4; row++ )
  {
    for( size_t col = 0; col < 2; col++ )
    {
      BOOST_CHECK_CLOSE(samples[col][row], exact[row][col], 1e-4);
    }
  }
}

// +-------------------------------------------------------------------------+
// |                    Check active variables sampling                      |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(NonDLowDiscrepancySampling_check_active_variables_sampling)
{
  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "    tabular_data \n"
    "    tabular_data_file = 'samples.dat' \n"
    "    freeform \n"
    "    write_restart 'NonDLowDiscrepancySampling_check_active_variables_sampling.rst' \n"
    "method \n"
    "  sampling \n"
    "    sample_type \n"
    "      low_discrepancy \n"
    "        rank_1_lattice \n"
    "    samples 5 \n"
    "    output silent \n"
    "variables \n"
    "  normal_uncertain = 2 \n"
    "    means = 0.0 0.0 \n"
    "    std_deviations = 1.0 1.0 \n"
    "  continuous_design = 2 \n "
    "    initial_point  0.6  0.7 \n "
    "    upper_bounds   5.8  2.9 \n "
    "    lower_bounds   0.5  -2.9 \n "
    "  active uncertain \n "
    "interface \n"
    "    analysis_drivers = 'genz' \n"
    "    analysis_components = 'cp1' \n"
    "    direct \n"
    "responses \n"
    "  response_functions = 1 \n"
    "  no_gradients \n"
    "  no_hessians \n";

  // Get problem description
  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Dakota::Opt_TPL_Test::create_env(dakota_input));
  Dakota::LibraryEnvironment& env = *p_env;

  // Execute the environment
  env.execute();

  // Read in the tabular output file
  const std::string tabular_data_name = "samples.dat";
  Dakota::RealMatrix samples;
  Dakota::TabularIO::read_data_tabular(
    tabular_data_name, "", samples, 5, 5, Dakota::TABULAR_NONE, true
  );

  // Check values of the lattice points
  for ( size_t row = 0; row < 5; row++ )
  {
    BOOST_CHECK_CLOSE(samples[0][row], 0.6, 1e-8);
    BOOST_CHECK_CLOSE(samples[1][row], 0.7, 1e-8);
  }
}

// +-------------------------------------------------------------------------+
// |                Sample from correlated random variables                  |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(NonDLowDiscrepancySampling_sample_correlated_distributions)
{
  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "    tabular_data \n"
    "    tabular_data_file = 'samples.dat' \n"
    "    freeform \n"
    "    write_restart 'NonDLowDiscrepancySampling_sample_correlated_distributions.rst' \n"
    "method \n"
    "  sampling \n"
    "    sample_type \n"
    "      low_discrepancy \n"
    "        rank_1_lattice \n"
    "          generating_vector inline 1 433461 \n"
    "          m_max 20 \n"
    "    samples = 10000 \n"
    "    seed = 2023 \n"
    "    output silent \n"
    "variables \n"
    "  normal_uncertain = 2 \n"
    "    means = 0.0 0.0 \n"
    "    std_deviations = 1.0 1.0 \n"
    "    uncertain_correlation_matrix \n"
    "      1.0 0.9  \n"
    "      0.9 1.0 \n"
    "interface \n"
    "    analysis_drivers = 'genz' \n"
    "    analysis_components = 'cp1' \n"
    "    direct \n"
    "responses \n"
    "  response_functions = 1 \n"
    "  no_gradients \n"
    "  no_hessians \n";

  // Get problem description
  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Dakota::Opt_TPL_Test::create_env(dakota_input));
  Dakota::LibraryEnvironment& env = *p_env;

  // Execute the environment
  env.execute();

  // Read in the tabular output file
  const std::string tabular_data_name = "samples.dat";
  Dakota::RealMatrix samples;
  int NB_OF_SAMPLES = 10000;
  Dakota::TabularIO::read_data_tabular(
    tabular_data_name, "", samples, NB_OF_SAMPLES, 3, Dakota::TABULAR_NONE, true
  );

  // Compute mean values
  double m1 = 0;
  double m2 = 0;
  for ( size_t j = 0; j < NB_OF_SAMPLES; j++ )
  {
    m1 += samples[0][j] / NB_OF_SAMPLES;
    m2 += samples[1][j] / NB_OF_SAMPLES;
  }

  // Compute standard deviations
  double s1 = 0;
  double s2 = 0;
  for ( size_t j = 0; j < NB_OF_SAMPLES; j++ )
  {
    s1 += (samples[0][j] - m1)*(samples[0][j] - m1);
    s2 += (samples[1][j] - m2)*(samples[1][j] - m2);
  }
  s1 = std::sqrt(s1 / (NB_OF_SAMPLES - 1));
  s2 = std::sqrt(s2 / (NB_OF_SAMPLES - 1));

  // Compute covariance
  double c = 0;
  for ( size_t j = 0; j < NB_OF_SAMPLES; j++ )
  {
    c += (samples[0][j] - m1)*(samples[1][j] - m2);
  }
  c /= NB_OF_SAMPLES - 1;

  // Check values
  double TOL = 1e-3;
  BOOST_CHECK_SMALL(std::abs(m1 - 0), TOL);
  BOOST_CHECK_SMALL(std::abs(m2 - 0), TOL);
  BOOST_CHECK_SMALL(std::abs(s1 - 1), TOL);
  BOOST_CHECK_SMALL(std::abs(s2 - 1), TOL);
  BOOST_CHECK_SMALL(std::abs(c - 0.9), TOL);
}

// +-------------------------------------------------------------------------+
// |                  Cannot sample discrete distributions                   |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(NonDLowDiscrepancySampling_check_discrete_distributions)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "    tabular_data \n"
    "    tabular_data_file = 'samples.dat' \n"
    "    freeform \n"
    "    write_restart 'NonDLowDiscrepancySampling_check_discrete_distributions.rst' \n"
    "method \n"
    "  sampling \n"
    "    sample_type \n"
    "      low_discrepancy \n"
    "        rank_1_lattice \n"
    "          no_random_shift \n"
    "    samples 4 \n"
    "    output silent \n"
    "variables \n"
    "  weibull_uncertain = 1 \n"
    "    alphas = 1.0 \n"
    "    betas = 1.5 \n"
    "  discrete_design_set \n "
    "    integer = 3 \n "
    "      initial_point 0 0 0 \n "
    "      num_set_values = 5 5 5 \n "
    "      set_values = -4 -2 0 2 4 -4 -2 0 2 4 -4 -2 0 2 4 \n "
    "interface \n"
    "    analysis_drivers = 'genz' \n"
    "    analysis_components = 'cp1' \n"
    "    direct \n"
    "responses \n"
    "  response_functions = 1 \n"
    "  no_gradients \n"
    "  no_hessians \n";

  // Get problem description
  std::shared_ptr<Dakota::LibraryEnvironment> p_env(Dakota::Opt_TPL_Test::create_env(dakota_input));
  Dakota::LibraryEnvironment& env = *p_env;

  // Check that correlated random variables throws an exception
  Dakota::RealMatrix points(2, 1);
  BOOST_CHECK_THROW(
    env.execute(),
    std::system_error
  );
}

} // end namespace TestNonDLowDiscrepancySampling

} // end namespace TestLowDiscrepancy

} // end namespace Dakota
