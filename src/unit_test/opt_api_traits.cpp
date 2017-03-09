
#include "dakota_data_util.hpp"

#include <limits>
#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;


//----------------------------------------------------------------

namespace {

  class dummyGradFreeOpt1 {

    public:

      struct Traits {
          typedef double scalarType;
          typedef std::vector<double> vectorType;
          static const bool supportsGradients           = false;
          static const bool supportsLinearEquality      = false;
          static const bool supportsLinearInequality    = false;
          static const bool supportsNonlinearEquality   = false;
          static const bool supportsNonlinearInequality = false;
          static const bool supportsScaling             = false;
          static const bool supportsLeastSquares        = false;
          static const bool supportsMultiobjectives     = false;
      };
  };

  //----------------------------------

  class dummyGradOpt1 {

    public:

      struct Traits {
          typedef double scalarType;
          typedef std::vector<double> vectorType;
          static const bool supportsGradients           = true;
          static const bool supportsLinearEquality      = true;
              struct LinearEquality
              {
                static const bool requiresBounds   = true;
                static const scalarType lowerBound = 0.0;
                static const scalarType upperBound = 1.0;
              };
          static const bool supportsLinearInequality    = false;
          static const bool supportsNonlinearEquality   = false;
          static const bool supportsNonlinearInequality = false;
          static const bool supportsScaling             = false;
          static const bool supportsLeastSquares        = false;
          static const bool supportsMultiobjectives     = false;
      };

  };
} // anonymous namespace

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_api_traits, test1)
{
  TEST_ASSERT( dummyGradOpt1::Traits::supportsLinearEquality );
  TEST_ASSERT( dummyGradOpt1::Traits::LinearEquality::requiresBounds );
  TEST_COMPARE( dummyGradOpt1::Traits::LinearEquality::lowerBound, <, dummyGradOpt1::Traits::LinearEquality::upperBound );

  // As an exmaple, this line won't compile and indicates early-on (at compile-time) that the
  // particular optimizer does not support a feature.
  //dummyGradFreeOpt1::Traits::scalarType lower_bound = dummyGradFreeOpt1::Traits::LinearEquality::lowerBound;

}

//----------------------------------------------------------------
