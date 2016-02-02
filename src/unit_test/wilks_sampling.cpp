
#include "NonDSampling.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;

namespace {

}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(wilks, num_samples1)
{
  std::map<int,int> gold_onesided;
  gold_onesided[50] = 14;
  gold_onesided[80] = 32;
  gold_onesided[90] = 45;
  gold_onesided[95] = 59;
  gold_onesided[99] = 90;

  Real alpha = 0.95;
  bool twosided = false;
  std::map<int,int>::const_iterator miter = gold_onesided.begin(),
                                     mend = gold_onesided.end();
  for( ; mend != miter; ++miter ) {
    Real beta = (Real)miter->first/100.0;
    int num_samples = NonDSampling::compute_wilks_sample_size(alpha, beta, twosided);
    TEST_EQUALITY( miter->second, num_samples );
  }


  std::map<int,int> gold_twosided;
  gold_twosided[50] = 34;
  gold_twosided[80] = 59;
  gold_twosided[90] = 77;
  gold_twosided[95] = 93;
  gold_twosided[99] = 130;

  twosided = true;
  miter = gold_twosided.begin();
  mend = gold_twosided.end();
  for( ; mend != miter; ++miter ) {
    Real beta = (Real)miter->first/100.0;
    int num_samples = NonDSampling::compute_wilks_sample_size(alpha, beta, twosided);
    TEST_EQUALITY( miter->second, num_samples );
  }

}
