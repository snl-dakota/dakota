
#include "opt_tpl_test_fixture.hpp"

#include "dakota_data_util.hpp"

#include <limits>
#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;


//----------------------------------------------------------------

namespace {

  const size_t VEC_SIZE = 5;
  const size_t MAT_ROWS = 3;

  class dummyTraits1 {
    public:
      typedef double scalarType;
      typedef std::vector<double> vectorType;
  };

  class dummyTraits2 {
    public:
      typedef float scalarType;
      typedef float vectorType[VEC_SIZE];
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

TEUCHOS_UNIT_TEST(opt_api_data_xfers, basic_vec1)
{
  RealVector vec(VEC_SIZE);
  vec.random();

  dummyTraits1::vectorType tpl1_vec;
  copy_data(vec, tpl1_vec);

  // Test correctness of size and values accurate to double precision
  TEST_EQUALITY( vec.length(), tpl1_vec.size() );
  double diff, max_diff = 0.0;
  for( size_t i=0; i<vec.length(); ++i )
    max_diff = (diff = std::fabs(vec[i]-tpl1_vec[i])) > max_diff ? diff : max_diff;
  max_diff += 1.0;
  Real real_tol = std::pow(10.0, -std::numeric_limits<dummyTraits1::scalarType>::digits10);
  TEST_FLOATING_EQUALITY( max_diff, 1.0, real_tol );

  dummyTraits2::vectorType tpl2_vec;
  copy_data<float>(vec, tpl2_vec, vec.length());

  // Test correctness of size and values accurate to single precision
  TEST_EQUALITY( vec.length(), VEC_SIZE );
  max_diff = 0.0;
  for( size_t i=0; i<vec.length(); ++i )
    max_diff = (diff = std::fabs(vec[i]-tpl2_vec[i])) > max_diff ? diff : max_diff;
  max_diff += 1.0;
  float float_tol = std::pow(10.0, -std::numeric_limits<dummyTraits2::scalarType>::digits10);
  TEST_FLOATING_EQUALITY( max_diff, 1.0, float_tol );
  TEST_COMPARE( max_diff, >, real_tol );
}

//----------------------------------------------------------------

