/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <exception>

#include "Rank1Lattice.hpp"
#include "low_discrepancy_data.hpp"

#include <fstream>
#include <cmath>

#include "opt_tpl_test.hpp"

#define BOOST_TEST_MODULE dakota_rank_1_lattice_test
#include <boost/test/included/unit_test.hpp>

namespace DakotaUnitTest {

namespace TestLowDiscrepancy {

//
//  Tests for rank-1 lattice rules
//
namespace TestRank1Lattice {

// +-------------------------------------------------------------------------+
// |                   Check values of the lattice points                    |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_points)
{
  
  // Get rank-1 lattice rule
  Dakota::Rank1Lattice lattice;
  lattice.no_random_shift();

  // Generate points of lattice rule
  size_t numPoints = 8;
  Dakota::RealMatrix points(2, numPoints);
  lattice.get_points(points);

  // Exact lattice points
  double exact[numPoints][2] = {
    {0, 0},
    {0.5, 0.5},
    {0.25, 0.75},
    {0.75, 0.25},
    {0.125, 0.375},
    {0.625, 0.875},
    {0.375, 0.125},
    {0.875, 0.625}
  };

  // Check values of the lattice points
  for ( size_t row = 0; row < numPoints; row++ )
  {
    for( size_t col = 0; col < 2; col++)
    {
      BOOST_CHECK_CLOSE(points[row][col], exact[row][col], 1e-4);
    }
  }
}

// +-------------------------------------------------------------------------+
// |                   mMax < 0 throws an exception                          |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_throws_mMax)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Define a generating vector
  Dakota::UInt32Vector generatingVector(
    Teuchos::View,
    Dakota::cools_kuo_nuyens_d250_m20,
    250
  );

  // Check that mMax < 1 throws an exception
  BOOST_CHECK_THROW(
    Dakota::Rank1Lattice(generatingVector, -1),
    std::system_error
  );
}

// +-------------------------------------------------------------------------+
// |                   dMax < 1 throws an exception                          |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_throws_dMax)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Define an emtpy generating vector
  Dakota::UInt32Vector generatingVector(
    Teuchos::View,
    Dakota::cools_kuo_nuyens_d250_m20,
    0
  );

  // Check that dMax < 1 throws an exception
  BOOST_CHECK_THROW(
    Dakota::Rank1Lattice(generatingVector, 20),
    std::system_error
  );
}

// +-------------------------------------------------------------------------+
// |                 seedValue < 0 throws an exception                       |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_throws_seedValue)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Check that dMax < 1 throws an exception
  BOOST_CHECK_THROW(
    Dakota::Rank1Lattice(-1),
    std::system_error
  );
}

// +-------------------------------------------------------------------------+
// |                           Tests for check_sizes                         |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_test_check_sizes)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Define a generating vector
  Dakota::UInt32Vector generatingVector(
    Teuchos::View,
    Dakota::cools_kuo_nuyens_d250_m20,
    10
  );

  /// Define a lattice
  Dakota::Rank1Lattice lattice(
    generatingVector,
    2,
    true,
    17,
    Dakota::RANK_1_LATTICE_RADICAL_INVERSE_ORDERING,
    Dakota::NORMAL_OUTPUT
  );

  /// Requesting points with dimension > dMax throws an exception
  Dakota::RealMatrix points(11, 2); /// 2 points in 11 dimensions
  BOOST_CHECK_THROW(
    lattice.get_points(points),
    std::system_error
  );

  /// But dimension == dMax is fine
  points.shape(10, 2); /// 2 points in 10 dimensions
  lattice.get_points(points);

  /// Requesting more than 2^mMax points throws an exception
  points.shape(2, 5); /// 5 points in 2 dimensions
  BOOST_CHECK_THROW(
    lattice.get_points(points),
    std::system_error
  );

  /// But 2^mMax points is fine
  points.shape(2, 4); /// 2 points in 4 dimensions
  lattice.get_points(points);

  /// Throws an error when number of columns of 'points' is not nMin - nMax
  points.shape(4, 2); /// 2 points in 4 dimensions
  BOOST_CHECK_THROW(
    lattice.get_points(0, 3, points),
    std::system_error
  );

  /// But nMin - nMax is fine
  lattice.get_points(0, 2, points);
}

// +-------------------------------------------------------------------------+
// |    Cannot provide `m_max` when specifying a default generating vector   |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_throws_mMax_in_input_file)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "method \n"
    "  sampling \n"
    "    sample_type low_discrepancy \n"
    "    rank_1_lattice \n"
    "        generating_vector predefined kuo \n"
    "        m_max 20 \n"
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

  // Check that this input file throws an exception
  BOOST_CHECK_THROW(
    Dakota::Opt_TPL_Test::create_env(dakota_input),
    std::system_error
  );
}

// +-------------------------------------------------------------------------+
// | Requesting more than the maximum number of samples throws an exception  |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_throws_max_num_samples_exceeded)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Define a generating vector
  Dakota::UInt32Vector generatingVector(
    Teuchos::View,
    Dakota::cools_kuo_nuyens_d250_m20,
    250
  );

  // Get a lattice rule assuming mostly 2^2 points
  Dakota::Rank1Lattice lattice(generatingVector, 2);

  // Check that requesting 5 points throws an exception
  Dakota::RealMatrix points(8, 5);
  BOOST_CHECK_THROW(
    lattice.get_points(points),
    std::system_error
  );
}

// +-------------------------------------------------------------------------+
// |            Requesting more dimensions throws an exception               |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_throws_max_dimension_exceeded)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Define a generating vector
  Dakota::UInt32Vector generatingVector(
    Teuchos::View,
    Dakota::cools_kuo_nuyens_d250_m20,
    6
  );

  // Get a lattice rule
  Dakota::Rank1Lattice lattice(generatingVector, 20);

  // Check that requesting a point in 7 dimensions throws an exception
  Dakota::RealMatrix points(7, 1);
  BOOST_CHECK_THROW(
    lattice.get_points(points),
    std::system_error
  );
}

// +-------------------------------------------------------------------------+
// |              Mismatched samples matrix throws an exception              |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_throws_mismatched_samples_matrix)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Get a lattice rule
  Dakota::Rank1Lattice lattice;

  // Check that a mismatched sample matrix throws an exception
  Dakota::RealMatrix points(32, 10);
  BOOST_CHECK_THROW(
    lattice.get_points(10, 21, points),
    std::system_error
  );
}

// +-------------------------------------------------------------------------+
// |                        Inline generating vector                         |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_inline_generating_vector)
{
  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "method \n"
    "  sampling \n"
    "    sample_type low_discrepancy \n"
    "    samples 32 \n"
    "    rank_1_lattice \n"
    "      generating_vector inline 1 182667 \n"
    "      m_max 20 \n"
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
}

// +-------------------------------------------------------------------------+
// |                      Generating vector from file                        |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_generating_vector_from_file)
{
  // Write a generating vector to file
  std::ofstream file;
  file.open("z.txt");
  file << "1\n182667";
  file.close();

  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "method \n"
    "  sampling \n"
    "    sample_type low_discrepancy \n"
    "    samples 16 \n"
    "    rank_1_lattice \n"
    "      generating_vector file 'z.txt' \n"
    "      m_max 20 \n"
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
}

// +-------------------------------------------------------------------------+
// |                            Integration test                             |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_lattice_rule_integration)
{
  // Get randomly-shifted rank-1 lattice rule with a fixed seed
  Dakota::Rank1Lattice lattice(17);

  // Get points from this lattice rule
  size_t numPoints = 1 << 15;
  size_t dimension = 4;
  Dakota::RealMatrix points(dimension, numPoints);
  lattice.get_points(points);

  // Compute integrand
  double integrand = 0;
  for ( size_t n = 0; n < numPoints; ++n )
  {
    double term = 0;
    for ( size_t d = 0; d < dimension; ++d )
    {
      term += (points[n][d] - 0.5)*(points[n][d] - 0.5);
    }
    integrand += std::exp(-term / 0.04);
  }
  integrand /= std::pow(0.2*std::sqrt(std::atan(1)*4), dimension);
  integrand /= numPoints;
  BOOST_CHECK_CLOSE(integrand, 0.998373, 1e-1);
}

// +-------------------------------------------------------------------------+
// |                              Annulus test                               |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_annulus_lattice_rules)
{
  
  // Get randomly-shifted rank-1 lattice rule with a fixed seed
  Dakota::Rank1Lattice lattice(17);

  // Get points from this lattice rule
  size_t numPoints = 1 << 16;
  size_t dimension = 2;
  Dakota::RealMatrix points(dimension, numPoints);
  lattice.get_points(points);

  // Compute integrand
  double integrand = 0;
  for ( size_t n = 0; n < numPoints; ++n )
  {
    double d = std::sqrt(points[n][0]*points[n][0] + points[n][1]*points[n][1]);
    double x_n = ( d > 0.2 and d < 0.45 ) ? 1 : 0;
    integrand = (integrand*n + x_n)/(n + 1);
  }
  BOOST_CHECK_CLOSE(4*integrand, 0.65*std::atan(1), 1e-1);
}

// +-------------------------------------------------------------------------+
// |                            Test random seed                             |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_random_seed_lattice)
{
  
  // Get randomly-shifted rank-1 lattice rule with seed 17
  Dakota::Rank1Lattice lattice_17a(17);

  // Get another randomly-shifted rank-1 lattice rule with seed 17
  Dakota::Rank1Lattice lattice_17b(17);

  // Get randomly-shifted rank-1 lattice rule with random seed
  Dakota::Rank1Lattice lattice_ra;

  // Get randomly-shifted rank-1 lattice rule with another random seed
  Dakota::Rank1Lattice lattice_rb;

  /// TODO: test seedValue 0

  // Generate points from these lattice rules
  size_t numPoints = 8;
  Dakota::RealMatrix points_17a(2, numPoints);
  lattice_17a.get_points(points_17a);
  Dakota::RealMatrix points_17b(2, numPoints);
  lattice_17b.get_points(points_17b);
  Dakota::RealMatrix points_ra(2, numPoints);
  lattice_ra.get_points(points_ra);
  Dakota::RealMatrix points_rb(2, numPoints);
  lattice_rb.get_points(points_rb);

  // Check values of the lattice points
  for ( size_t row = 0; row < numPoints; row++ )
  {
    for( size_t col = 0; col < 2; col++ )
    {
      // Lattice rules with same random seed should give the same points
      BOOST_CHECK_CLOSE(points_17a[row][col], points_17b[row][col], 1e-4);
      // "Random" seed should give different points
      BOOST_CHECK_NE(points_17a[row][col], points_ra[row][col]);
      // Another "random" seed should give different points again
      BOOST_CHECK_NE(points_ra[row][col], points_rb[row][col]);
    }
  }
}

// +-------------------------------------------------------------------------+
// |                            Test randomization                           |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_randomization)
{
  // Get rank-1 lattice rule
  Dakota::Rank1Lattice lattice;

  // Generate points from this lattice rule
  size_t dimension = 10;
  size_t numPoints = 8;
  Dakota::RealMatrix points(dimension, numPoints);
  lattice.get_points(points);

  // Randomize the lattice
  lattice.randomize();

  // Generate another set of points from this lattice rule
  Dakota::RealMatrix different_points(dimension, numPoints);
  lattice.get_points(different_points);

  // Check if the lattice points are different
  for ( size_t row = 0; row < numPoints; row++ )
  {
    for( size_t col = 0; col < dimension; col++ )
    {
      BOOST_CHECK_NE(points[row][col], different_points[row][col]);
    }
  }
}

// +-------------------------------------------------------------------------+
// |                       Test disable randomization                        |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_disabled_randomization)
{
  // Get rank-1 lattice rule
  Dakota::Rank1Lattice lattice;
  lattice.no_random_shift();

  // Generate points from this lattice rule
  size_t dimension = 2;
  size_t numPoints = 3;
  Dakota::RealMatrix points(dimension, numPoints);
  lattice.get_points(points);

  // Exact lattice points
  double exact[numPoints][2] = {
    {0, 0},
    {0.5, 0.5},
    {0.25, 0.75}
  };

  // Check values of the lattice points
  for ( size_t row = 0; row < numPoints; row++ )
  {
    for( size_t col = 0; col < dimension; col++)
    {
      BOOST_CHECK_CLOSE(points[row][col], exact[row][col], 1e-4);
    }
  }
  
}

// +-------------------------------------------------------------------------+
// |                              Test seed 0                                |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_lattice_seed_0)
{
  
  // Get randomly-shifted rank-1 lattice rule with seed 0
  Dakota::Rank1Lattice lattice_0a(0);

  // Get another randomly-shifted rank-1 lattice rule with seed 0
  Dakota::Rank1Lattice lattice_0b(0);

  // Get another rank-1 lattice rule with no random shift
  Dakota::Rank1Lattice lattice;
  lattice.no_random_shift();

  // Generate points from these lattice rules
  size_t numPoints = 8;
  size_t dimension = 13;
  Dakota::RealMatrix points_0a(dimension, numPoints);
  lattice_0a.get_points(points_0a);
  Dakota::RealMatrix points_0b(dimension, numPoints);
  lattice_0b.get_points(points_0b);
  Dakota::RealMatrix points(dimension, numPoints);
  lattice.get_points(points);

  // Check values of the lattice points
  for ( size_t row = 0; row < numPoints; row++ )
  {
    for( size_t col = 0; col < dimension; col++ )
    {
      /// Check that seed 0 (a & b) generate the same points
      BOOST_CHECK_CLOSE(points_0a[row][col], points_0b[row][col], 1e-4);
      /// Check that is is different from a lattice with no random shift
      BOOST_CHECK_NE(points_0a[row][col], points[row][col]);
    }
  }
}

// +-------------------------------------------------------------------------+
// |                            Test negative seed                           |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(lattice_check_lattice_negative_seed)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  BOOST_CHECK_THROW(
    Dakota::Rank1Lattice lattice(-1),
    std::system_error
  );
}

} // end namespace TestRank1Lattice

} // end namespace TestLowDiscrepancy

} // end namespace Dakota