/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <exception>

#include "DigitalNet.hpp"

#include <fstream>
#include <cmath>
#include <ciso646>
#include "opt_tpl_test.hpp"

#define BOOST_TEST_MODULE dakota_digital_net_test
#include <boost/test/included/unit_test.hpp>

namespace DakotaUnitTest {

namespace TestLowDiscrepancy {

//
//  Tests for DigitalNet
//
namespace TestDigitalNet {

// +-------------------------------------------------------------------------+
// |                         Test digital net points                         |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_points)
{
  // Generating matrices
  Dakota::UInt64 C[][2] = {
    { 1, 1 },
    { 2, 3 },
    { 4, 5 },
    { 8, 15},
    {16, 17}
  };

  // Create digital net
  Dakota::DigitalNet digital_net(
    Dakota::UInt64Matrix(Teuchos::View, &C[0][0], 2, 2, 5),
    5,
    5,
    5,
    false,
    false,
    Dakota::generate_system_seed(),
    Dakota::DIGITAL_NET_GRAY_CODE_ORDERING,
    false,
    Dakota::NORMAL_OUTPUT
  );

  // Get digital net points
  size_t numPoints = 32;
  Dakota::RealMatrix points(2, numPoints);
  digital_net.get_points(points);

  // Exact lattice points
  double exact[][2] = {
    {0, 0},
    {0.5, 0.5},
    {0.75, 0.25},
    {0.25, 0.75},
    {0.375, 0.375},
    {0.875, 0.875},
    {0.625, 0.125},
    {0.125, 0.625},
    {0.1875, 0.3125},
    {0.6875, 0.8125},
    {0.9375, 0.0625},
    {0.4375, 0.5625},
    {0.3125, 0.1875},
    {0.8125, 0.6875},
    {0.5625, 0.4375},
    {0.0625, 0.9375},
    {0.09375, 0.46875},
    {0.59375, 0.96875},
    {0.84375, 0.21875},
    {0.34375, 0.71875},
    {0.46875, 0.09375},
    {0.96875, 0.59375},
    {0.71875, 0.34375},
    {0.21875, 0.84375},
    {0.15625, 0.15625},
    {0.65625, 0.65625},
    {0.90625, 0.40625},
    {0.40625, 0.90625},
    {0.28125, 0.28125},
    {0.78125, 0.78125},
    {0.53125, 0.03125},
    {0.03125, 0.53125}
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
// |                         Test digital net 'mMax'                         |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_mMax)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Generating matrices
  Dakota::UInt64 C[][2] = {
    { 1, 1 },
    { 2, 3 },
    { 4, 5 },
    { 8, 15},
    {16, 17}
  };

  // Check that digital net with mMax < 1 is invalid
  BOOST_CHECK_THROW(
    Dakota::DigitalNet digital_net(
      Dakota::UInt64Matrix(Teuchos::View, &C[0][0], 2, 2, 0),
      0, // mMax
      5  // tMax
    ),
    std::system_error
  );

  // Check that digital net with mMax different from number of columns in the
  // generating matrices is invalid
  BOOST_CHECK_THROW(
    Dakota::DigitalNet digital_net(
      Dakota::UInt64Matrix(Teuchos::View, &C[0][0], 2, 2, 5),
      4, // mMax
      5  // tMax
    ),
    std::system_error
  );
}

// +-------------------------------------------------------------------------+
// |                         Test digital net 'dMax'                         |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_dMax)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Generating matrices
  Dakota::UInt64 C[][2] = {
    { 1, 1 },
    { 2, 3 },
    { 4, 5 },
    { 8, 15},
    {16, 17}
  };

  // Check that digital net with dMax < 1 is invalid
  BOOST_CHECK_THROW(
    Dakota::DigitalNet digital_net(
      Dakota::UInt64Matrix(Teuchos::View, &C[0][0], 0, 0, 5),
      5, // mMax
      5  // tMax
    ),
    std::system_error
  );
}

// +-------------------------------------------------------------------------+
// |                           Test digital shift                            |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_digital_shift)
{
  // Generating matrices
  Dakota::UInt64 C[][2] = {
    { 1, 1 },
    { 2, 3 },
    { 4, 5 },
    { 8, 15},
    {16, 17}
  };

  // Create digital net
  Dakota::DigitalNet digital_net(
    Dakota::UInt64Matrix(Teuchos::View, &C[0][0], 2, 2, 5),
    5,
    5
  );
  digital_net.no_scrambling();

  // Generate points
  int numPoints = 4;
  Dakota::RealMatrix points(2, numPoints);
  digital_net.get_points(points);

  // Create digital net without digital shift
  digital_net.no_digital_shift();

  // Generate points
  Dakota::RealMatrix points_no_shift(2, numPoints);
  digital_net.get_points(points_no_shift);

  // Check that the points generated by the digital net with and without 
  // digital shift are different
  for ( size_t row = 0; row < 2; row++ )
  {
    for( size_t col = 0; col < numPoints; col++)
    {
      BOOST_CHECK_NE(points(row, col), points_no_shift(row, col));
    }
  }

  // Exact lattice points
  double exact[][2] = {
    {0, 0},
    {0.5, 0.5},
    {0.75, 0.25},
    {0.25, 0.75},
  };

  // Check that the points generated by the unshifted, unscrambled digital net
  // are correct
  for ( size_t row = 0; row < numPoints; row++ )
  {
    for( size_t col = 0; col < 2; col++)
    {
      BOOST_CHECK_CLOSE(points_no_shift[row][col], exact[row][col], 1e-4);
    }
  }
}

// +-------------------------------------------------------------------------+
// |                      Test digital net scrambling                        |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_scrambling)
{
  // Generating matrices
  Dakota::UInt64 C[][2] = {
    { 1, 1 },
    { 2, 3 },
    { 4, 5 },
    { 8, 15},
    {16, 17}
  };

  // Create digital net
  Dakota::DigitalNet digital_net(
    Dakota::UInt64Matrix(Teuchos::View, &C[0][0], 2, 2, 5),
    5,
    5
  );

  // Generate points
  int numPoints = 32;
  Dakota::RealMatrix points(2, numPoints);
  digital_net.get_points(points);

  // Create digital net without scrambling
  digital_net.no_scrambling();

  // Generate points
  Dakota::RealMatrix points_no_scrambling(2, numPoints);
  digital_net.get_points(points_no_scrambling);

  // Check that the points generated by the digital net with and without 
  // scrambling are different
  // NOTE: can't use 'BOOST_CHECK_NE' as some points may be the same
  bool all_same = true;
  for ( size_t row = 0; row < 2; row++ )
  {
    for( size_t col = 1; col < numPoints; col++)
    {
      if ( points(row, col) != points_no_scrambling(row, col) )
      {
        all_same = false;
      }
    }
  }
  BOOST_CHECK(!all_same);
}

// +-------------------------------------------------------------------------+
// |                         Test digital net seed                           |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_digital_net_seed)
{
  // Make sure an exception is thrown instead of an exit code
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // Negative seed is not accepted
  BOOST_CHECK_THROW(
    Dakota::DigitalNet digital_net(-1),
    std::system_error
  );

  // Get digital_net rule with seed 0
  Dakota::DigitalNet digital_net_0a(0);

  // Get another digital_net with seed 0
  Dakota::DigitalNet digital_net_0b(0);

  // Get another digital_net with no digital shift
  Dakota::DigitalNet digital_net;
  digital_net.no_digital_shift();

  // Generate points from these digital net
  size_t numPoints = 8;
  size_t dimension = 13;
  Dakota::RealMatrix points_0a(dimension, numPoints);
  digital_net_0a.get_points(points_0a);
  Dakota::RealMatrix points_0b(dimension, numPoints);
  digital_net_0b.get_points(points_0b);
  Dakota::RealMatrix points(dimension, numPoints);
  digital_net.get_points(points);

  // Check values of the digital net points
  for ( size_t row = 0; row < numPoints; row++ )
  {
    for( size_t col = 0; col < dimension; col++ )
    {
      /// Check that seed 0 (a & b) generate the same points
      BOOST_CHECK_CLOSE(points_0a[row][col], points_0b[row][col], 1e-4);
      /// Check that is is different from a digital net withoug digital shift
      BOOST_CHECK_NE(points_0a[row][col], points[row][col]);
    }
  }
}

// +-------------------------------------------------------------------------+
// |                     Test digital net natural ordering                   |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_natural_ordering)
{
  // Generating matrices
  Dakota::UInt64 C[][2] = {
    { 1, 1},
    { 2, 3},
    { 4, 5},
    { 8, 15},
    {16, 17}
  };

  // Create digital net with natural ordering
  Dakota::DigitalNet digital_net(
    Dakota::UInt64Matrix(Teuchos::View, &C[0][0], 2, 2, 5),
    5,
    5,
    5,
    false,
    false,
    Dakota::generate_system_seed(),
    Dakota::DIGITAL_NET_NATURAL_ORDERING,
    false,
    Dakota::NORMAL_OUTPUT
  );

  // Check that requesting a number of points that is not a power of 2 
  // throws an error
  Dakota::RealMatrix points(2, 7);
  BOOST_CHECK_THROW(
    digital_net.get_points(points),
    std::system_error
  );

  // But a number of points that is a power of 2 is fine
  points.shape(2, 8);
  digital_net.get_points(points);

  // Check that none of the points are 0 (except the first point)
  for ( size_t row = 0; row < 2; row++ )
  {
    for( size_t col = 1; col < 8; col++)
    {
      BOOST_CHECK_NE(points(row, col), 0);
    }
  }
}

// +-------------------------------------------------------------------------+
// |               Test digital net inline generating matrices               |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_inline_generating_matrices)
{
  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "method \n"
    "  sampling \n"
    "    sample_type \n"
    "      low_discrepancy \n"
    "        digital_net \n"
    "          generating_matrices inline \n"
    "            1 2 4 8 16 \n"
    "            1 3 5 15 17 \n"
    "          m_max 5 \n"
    "          t_max 5 \n"
    "    samples 32 \n"
    "  output silent \n"
    "variables \n"
    "  uniform_uncertain = 2 \n"
    "    lower_bounds = -1.0 -1.0 \n"
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
// |           Test digital net generating matrices read from file           |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_generating_matrices_from_file)
{
  // Write a generating vector to file
  std::ofstream file;
  file.open("C.txt");
  file << "1 2 4 8 16 \n 1 3 5 15 17";
  file.close();

  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "method \n"
    "  sampling \n"
    "    sample_type \n"
    "      low_discrepancy \n"
    "        digital_net \n"
    "          generating_matrices file 'C.txt' \n"
    "          m_max 5 \n"
    "          t_max 5 \n"
    "    samples 32 \n"
    "  output silent \n"
    "variables \n"
    "  uniform_uncertain = 2 \n"
    "    lower_bounds = -1.0 -1.0 \n"
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
// |               Test digital net default generating matrices              |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_default_generating_matrices)
{
  // Example dakota input specification
  char dakota_input[] =
    "environment \n"
    "method \n"
    "  sampling \n"
    "    sample_type \n"
    "      low_discrepancy \n"
    "        sobol \n"
    "          generating_matrices predefined sobol_order_2 \n"
    "    samples 1024 \n"
    "  output silent \n"
    "variables \n"
    "  normal_uncertain = 2 \n"
    "    means = 0.0 0.0 \n"
    "    std_deviations = 1.0 1.0 \n"
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
// |                  Test digital net least significant bit                 |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_least_significant_bit_first)
{
  // Generating matrices
  Dakota::UInt64 C[][2] = {
    { 1, 1},
    { 2, 3},
    { 4, 5},
    { 8, 15},
    {16, 17}
  };

  // Create digital net
  Dakota::DigitalNet digital_net_lsb(
    Dakota::UInt64Matrix(Teuchos::View, &C[0][0], 2, 2, 5),
    5,
    5
  );
  digital_net_lsb.no_digital_shift();
  digital_net_lsb.no_scrambling();

  // Radical inverse generating matrices
  Dakota::UInt64 C_inv[][2] = {
    // {0x8000000000000000, 0x8000000000000000},
    // {0x4000000000000000, 0xc000000000000000},
    // {0x2000000000000000, 0xa000000000000000},
    // {0x1000000000000000, 0xf000000000000000},
    // { 0x800000000000000, 0x8800000000000000}
    {16, 16},
    {8, 24},
    {4, 20},
    {2, 30},
    {1, 17},
  };

  // Create digital net with most significant bit first
  Dakota::DigitalNet digital_net_msb(
    Dakota::UInt64Matrix(Teuchos::View, &C_inv[0][0], 2, 2, 5),
    5,
    5,
    5,
    false,
    false,
    0,
    Dakota::DIGITAL_NET_GRAY_CODE_ORDERING,
    true,
    Dakota::NORMAL_OUTPUT
  );

  // Generate points from both sequences
  int dimension = 2;
  int numPoints = 32;
  Dakota::RealMatrix points_lsb(dimension, numPoints);
  digital_net_lsb.get_points(points_lsb);
  Dakota::RealMatrix points_msb(dimension, numPoints);
  digital_net_msb.get_points(points_msb);

  // Check that the points are the same
  for ( size_t row = 0; row < numPoints; row++ )
  {
    for( size_t col = 0; col < dimension; col++ )
    {
      BOOST_CHECK_CLOSE(points_lsb[row][col], points_msb[row][col], 1e-4);
    }
  }
}

// +-------------------------------------------------------------------------+
// |                      Test digital net integration                       |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_integration_test)
{
  // Get digital net with a fixed seed
  Dakota::DigitalNet digital_net(19);

  // Get points from this digital net
  size_t numPoints = 1 << 15;
  size_t dimension = 4;
  Dakota::RealMatrix points(dimension, numPoints);
  digital_net.get_points(points);

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
// |                     Annulus test for digital nets                       |
// +-------------------------------------------------------------------------+
BOOST_AUTO_TEST_CASE(digital_net_check_annulus)
{

  // Get digital_net rule with a fixed seed
  Dakota::DigitalNet digital_net(17);

  // Get points from this lattice rule
  size_t numPoints = 1 << 16;
  size_t dimension = 2;
  Dakota::RealMatrix points(dimension, numPoints);
  digital_net.get_points(points);

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

} // end namespace TestDigitalNet

} // end namespace TestLowDiscrepancy

} // end namespace Dakota
