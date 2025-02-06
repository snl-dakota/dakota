/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "opt_tpl_test_fixture.hpp"

#include "dakota_data_util.hpp"

#include <limits>
#include <string>
#include <map>

#include <gtest/gtest.h>

using namespace Dakota;

//----------------------------------------------------------------

namespace {

  const size_t VEC_SIZE = 5;
  const size_t MAT_ROWS = 3;

  class dummyGradFreeOpt1 {

    public:

      struct Traits {
          typedef double scalarType;
          typedef std::vector<double> vectorType;
          static const bool supportsGradients = false;
      };
  };

  class dummyGradFreeOpt2 {

    public:

      struct Traits {
          typedef float scalarType;
          typedef float vectorType[VEC_SIZE];
          static const bool supportsGradients = false;
      };

  };

} // anonymous namespace

//----------------------------------------------------------------

namespace {

  template <typename ScalarType>
  void copy_data( const RealVector& vec,
                        ScalarType* ptr, 
                        size_t ptr_len )
  {
    if (ptr_len != vec.length()) { // could use <, but enforce exact match
      Cerr << "Error: bad ptr_len in copy_data(Dakota::Array<T>, T* ptr)."
        << std::endl;
      abort_handler(-1);
    }
    for (size_t i=0; i<ptr_len; ++i)
      ptr[i] = vec[i];
  }

} // anonymous namespace

//----------------------------------------------------------------

TEST(opt_api_core_data_xfers_tests, test_opt_api_data_xfers_basic_vec1)
{
  RealVector vec(VEC_SIZE);
  vec.random();

  dummyGradFreeOpt1::Traits::vectorType tpl1_vec;
  copy_data(vec, tpl1_vec);

  // Test correctness of size and values accurate to double precision
  EXPECT_TRUE(( vec.length() == tpl1_vec.size() ));
  double diff, max_diff = 0.0;
  for( size_t i=0; i<vec.length(); ++i )
    max_diff = (diff = std::fabs(vec[i]-tpl1_vec[i])) > max_diff ? diff : max_diff;
  max_diff += 1.0;
  Real real_tol = std::pow(10, -std::numeric_limits<dummyGradFreeOpt1::Traits::scalarType>::digits10);
  EXPECT_LT(std::fabs(1. - max_diff / 1.0), real_tol/100. );

  dummyGradFreeOpt2::Traits::vectorType tpl2_vec;
  copy_data<float>(vec, tpl2_vec, vec.length());

  // Test correctness of size and values accurate to single precision
  EXPECT_TRUE(( vec.length() == VEC_SIZE ));
  max_diff = 0.0;
  for( size_t i=0; i<vec.length(); ++i )
    max_diff = (diff = std::fabs(vec[i]-tpl2_vec[i])) > max_diff ? diff : max_diff;
  max_diff += 1.0;
  float float_tol = 100.0*std::pow(10, -std::numeric_limits<dummyGradFreeOpt2::Traits::scalarType>::digits10);
  EXPECT_LT(std::fabs(1. - max_diff / 1.0), float_tol/100. );
  EXPECT_GT( max_diff, real_tol );
}

//----------------------------------------------------------------

TEST(opt_api_core_data_xfers_tests, test_opt_api_data_xfers_basic_mat1)
{
  const int MAT_ROW = 1;
  RealMatrix mat(MAT_ROWS, VEC_SIZE);
  mat.random();

  dummyGradFreeOpt1::Traits::vectorType tpl1_vec;
  copy_row_vector(mat, MAT_ROW, tpl1_vec);

  // Test correctness of size and values accurate to double precision
  EXPECT_TRUE(( mat.numCols() == tpl1_vec.size() ));
  double diff, max_diff = 0.0;
  for( size_t i=0; i<tpl1_vec.size(); ++i )
    max_diff = (diff = std::fabs(mat(MAT_ROW,i)-tpl1_vec[i])) > max_diff ? diff : max_diff;
  max_diff += 1.0;
  Real real_tol = std::pow(10, -std::numeric_limits<dummyGradFreeOpt1::Traits::scalarType>::digits10);
  EXPECT_LT(std::fabs(1. - max_diff / 1.0), real_tol/100. );


  RealMatrix mat2;
  insert_row_vector(tpl1_vec, 4, mat2);

  // Test that matrix gets resized correctly and values are correnct in the propoer row
  EXPECT_TRUE(( mat2.numRows() == 5 ));
  EXPECT_TRUE(( mat2.numCols() == VEC_SIZE ));
  max_diff = 0.0;
  for( size_t i=0; i<tpl1_vec.size(); ++i )
    max_diff = (diff = std::fabs(mat2(4,i)-tpl1_vec[i])) > max_diff ? diff : max_diff;
  max_diff += 1.0;
  EXPECT_LT(std::fabs(1. - max_diff / 1.0), real_tol/100. );
}

//----------------------------------------------------------------

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
