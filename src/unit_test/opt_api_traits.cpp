
#include "dakota_data_util.hpp"
#include "DakotaOptTraits.hpp"

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

namespace {

  struct DummyTraits1 : public InheritableSingleton_OptTraits
  {
    // -------------------------
    // This is boilerplate that everyone would be expected to do
    // and could later be captured in a macro
    // -------------------------
    ~DummyTraits1() { }

    // Enforce a single object
    static InheritableSingleton_OptTraits * instance()
    {
      static DummyTraits1 * this_instance = new DummyTraits1();
      return this_instance;
    }

   private:

    // Disallow multiple objects
    DummyTraits1() :
      InheritableSingleton_OptTraits()
    { }

    DummyTraits1(const DummyTraits1&); // don't implement
    DummyTraits1 & operator =(const DummyTraits1&); // don't implement

    // This is needed of we choose pure virtual in the base traits class
    //bool is_derived() { return true; }

    // -------------------------
    // End of boilerplate
    // -------------------------

   public:

    // Customize behavior via inheritance
    bool requires_bounds() { return true; }

  };
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(opt_api_traits, test2)
{
  InheritableSingleton_OptTraits * base_traits1 = new InheritableSingleton_OptTraits();
  InheritableSingleton_OptTraits * base_traits2 = new InheritableSingleton_OptTraits();

  TEST_ASSERT( base_traits1->requires_bounds() == false );
  TEST_ASSERT( base_traits2->requires_bounds() == false );
  // These are different and distinct objects
  TEST_ASSERT( base_traits1 != base_traits2 );

  InheritableSingleton_OptTraits * derived_traits1 = DummyTraits1::instance();
  InheritableSingleton_OptTraits * derived_traits2 = DummyTraits1::instance();
  TEST_ASSERT( derived_traits1->requires_bounds() == true );
  // These are the same object 
  TEST_ASSERT( derived_traits1 == derived_traits2 );
}

//----------------------------------------------------------------
