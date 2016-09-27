
#include "NonDSampling.hpp"

#include <string>
#include <map>

#include <Teuchos_UnitTestHarness.hpp> 


using namespace Dakota;


//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(wilks, firstorder)
{
  /*

    The following gold results come from 

      "GRS Method for Uncertainty and Sensitivity Evaluation of Code Results and Applications," Horst Glaeser, NURETH-14

      See slide 8.

   * */

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
    int num_samples = NonDSampling::compute_wilks_sample_size(1, alpha, beta, twosided);
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
    int num_samples = NonDSampling::compute_wilks_sample_size(1, alpha, beta, twosided);
    TEST_EQUALITY( miter->second, num_samples );
  }
}

//----------------------------------------------------------------

namespace {
  std::map<int, std::map<int,int> >
  test_order_table(unsigned short order, bool twosided, const std::map<int, std::map<int,int> > & gold_table_data)
  {
    std::map<int, std::map<int,int> > computed_table_data;

    int  nalpha, nbeta;
    Real alpha, beta;

    std::map<int, std::map<int,int> >::const_iterator iter = gold_table_data.begin(),
                                                  iter_end = gold_table_data.end();
    for( ; iter_end != iter; ++iter ) {
      nbeta = iter->first;
      if( nbeta> 100 )
        beta = (Real)nbeta/1000.0;
      else
        beta = (Real)nbeta/100.0;

      std::map<int,int>::const_iterator miter = iter->second.begin(),
                                         mend = iter->second.end();
      for( ; mend != miter; ++miter ) {
        nalpha = miter->first;
        if( nalpha > 100 )
          alpha = (Real)nalpha/1000.0;
        else
          alpha = (Real)nalpha/100.0;

        computed_table_data[nbeta][nalpha] = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);
      }
    }
    return computed_table_data;
  }
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(wilks, onesided_firstorder)
{
  /* 
   *
   * These data come from https://www.oecd-nea.org/nsd/docs/2013/csni-r2013-8-part2.pdf
   * pg. 22, Table 1
   *
   * */
  std::map<int, std::map<int,int> > gold_firstorder;
  gold_firstorder[95 ][95] = 59 ;  gold_firstorder[96 ][95] = 63 ;  gold_firstorder[97 ][95] = 69 ;
  gold_firstorder[95 ][96] = 74 ;  gold_firstorder[96 ][96] = 79 ;  gold_firstorder[97 ][96] = 86 ;
  gold_firstorder[95 ][97] = 99 ;  gold_firstorder[96 ][97] = 106;  gold_firstorder[97 ][97] = 116;
  gold_firstorder[95 ][98] = 149;  gold_firstorder[96 ][98] = 160;  gold_firstorder[97 ][98] = 174;
  gold_firstorder[95 ][99] = 299;  gold_firstorder[96 ][99] = 321;  gold_firstorder[97 ][99] = 349;
                                 
  gold_firstorder[98 ][95] = 77 ;  gold_firstorder[99 ][95] = 90 ;  gold_firstorder[995][95] = 104;
  gold_firstorder[98 ][96] = 96 ;  gold_firstorder[99 ][96] = 113;  gold_firstorder[995][96] = 130;
  gold_firstorder[98 ][97] = 129;  gold_firstorder[99 ][97] = 152;  gold_firstorder[995][97] = 174;
  gold_firstorder[98 ][98] = 194;  gold_firstorder[99 ][98] = 228;  gold_firstorder[995][98] = 263;
  gold_firstorder[98 ][99] = 390;  gold_firstorder[99 ][99] = 459;  gold_firstorder[995][99] = 528;

  gold_firstorder[999][95] = 135;
  gold_firstorder[999][96] = 170;
  gold_firstorder[999][97] = 227;
  gold_firstorder[999][98] = 342;
  gold_firstorder[999][99] = 688;


  std::map<int, std::map<int,int> > computed_table_data = test_order_table(1, false, gold_firstorder);
  
  int nalpha, nbeta;
  std::map<int, std::map<int,int> >::const_iterator iter = gold_firstorder.begin(),
                                                iter_end = gold_firstorder.end();
  for( ; iter_end != iter; ++iter ) {
      nbeta = iter->first;
      std::map<int,int>::const_iterator miter = iter->second.begin(),
                                         mend = iter->second.end();
    for( ; mend != miter; ++miter ) {
      nalpha = miter->first;
      TEST_EQUALITY( gold_firstorder[nbeta][nalpha], computed_table_data[nbeta][nalpha] );
    }
  }
}


//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(wilks, onesided_secondorder)
{
  /* 
   *
   * These data come from https://www.oecd-nea.org/nsd/docs/2013/csni-r2013-8-part2.pdf
   * pg. 22, Table 2
   *
   * */
  std::map<int, std::map<int,int> > gold_secondorder;
  gold_secondorder[95 ][95] = 93 ;  gold_secondorder[96 ][95] = 99 ;  gold_secondorder[97 ][95] = 105;
  gold_secondorder[95 ][96] = 117;  gold_secondorder[96 ][96] = 124;  gold_secondorder[97 ][96] = 132;
  gold_secondorder[95 ][97] = 157;  gold_secondorder[96 ][97] = 166;  gold_secondorder[97 ][97] = 177;
  gold_secondorder[95 ][98] = 236;  gold_secondorder[96 ][98] = 249;  gold_secondorder[97 ][98] = 266;
  gold_secondorder[95 ][99] = 473;  gold_secondorder[96 ][99] = 500;  gold_secondorder[97 ][99] = 534;
                                 
  gold_secondorder[98 ][95] = 115;  gold_secondorder[99 ][95] = 130;  gold_secondorder[995][95] = 146;
  gold_secondorder[98 ][96] = 144;  gold_secondorder[99 ][96] = 164;  gold_secondorder[995][96] = 183;
  gold_secondorder[98 ][97] = 193;  gold_secondorder[99 ][97] = 219;  gold_secondorder[995][97] = 245;
  gold_secondorder[98 ][98] = 290;  gold_secondorder[99 ][98] = 330;  gold_secondorder[995][98] = 369;
  gold_secondorder[98 ][99] = 581;  gold_secondorder[99 ][99] = 662;  gold_secondorder[995][99] = 740;

  gold_secondorder[999][95] = 181;
  gold_secondorder[999][96] = 227;
  gold_secondorder[999][97] = 304;
  gold_secondorder[999][98] = 458;
  gold_secondorder[999][99] = 920;


  std::map<int, std::map<int,int> > computed_table_data = test_order_table(2, false, gold_secondorder);
  
  int nalpha, nbeta;
  std::map<int, std::map<int,int> >::const_iterator iter = gold_secondorder.begin(),
                                                iter_end = gold_secondorder.end();
  for( ; iter_end != iter; ++iter ) {
      nbeta = iter->first;
      std::map<int,int>::const_iterator miter = iter->second.begin(),
                                         mend = iter->second.end();
    for( ; mend != miter; ++miter ) {
      nalpha = miter->first;
      TEST_EQUALITY( gold_secondorder[nbeta][nalpha], computed_table_data[nbeta][nalpha] );
    }
  }
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(wilks, onesided_thirdorder)
{
  /* 
   *
   * These data come from https://www.oecd-nea.org/nsd/docs/2013/csni-r2013-8-part2.pdf
   * pg. 22, Table 3
   *
   * */
  std::map<int, std::map<int,int> > gold_thirdorder;
  gold_thirdorder[95 ][95] = 124;  gold_thirdorder[96 ][95] = 130;  gold_thirdorder[97 ][95] = 138;
  gold_thirdorder[95 ][96] = 156;  gold_thirdorder[96 ][96] = 163;  gold_thirdorder[97 ][96] = 173;
  gold_thirdorder[95 ][97] = 208;  gold_thirdorder[96 ][97] = 218;  gold_thirdorder[97 ][97] = 231;
  gold_thirdorder[95 ][98] = 313;  gold_thirdorder[96 ][98] = 328;  gold_thirdorder[97 ][98] = 347;
  gold_thirdorder[95 ][99] = 628;  gold_thirdorder[96 ][99] = 658;  gold_thirdorder[97 ][99] = 696;
                                 
  gold_thirdorder[98 ][95] = 148;  gold_thirdorder[99 ][95] = 165;  gold_thirdorder[995][95] = 182;
  gold_thirdorder[98 ][96] = 186;  gold_thirdorder[99 ][96] = 207;  gold_thirdorder[995][96] = 229;
  gold_thirdorder[98 ][97] = 248;  gold_thirdorder[99 ][97] = 277;  gold_thirdorder[995][97] = 306;
  gold_thirdorder[98 ][98] = 374;  gold_thirdorder[99 ][98] = 418;  gold_thirdorder[995][98] = 461;
  gold_thirdorder[98 ][99] = 749;  gold_thirdorder[99 ][99] = 838;  gold_thirdorder[995][99] = 924;

  gold_thirdorder[999][95] = 220 ;
  gold_thirdorder[999][96] = 277 ;
  gold_thirdorder[999][97] = 370 ;
  gold_thirdorder[999][98] = 557 ;
  gold_thirdorder[999][99] = 1119;


  std::map<int, std::map<int,int> > computed_table_data = test_order_table(3, false, gold_thirdorder);
  
  int nalpha, nbeta;
  std::map<int, std::map<int,int> >::const_iterator iter = gold_thirdorder.begin(),
                                                iter_end = gold_thirdorder.end();
  for( ; iter_end != iter; ++iter ) {
      nbeta = iter->first;
      std::map<int,int>::const_iterator miter = iter->second.begin(),
                                         mend = iter->second.end();
    for( ; mend != miter; ++miter ) {
      nalpha = miter->first;
      TEST_EQUALITY( gold_thirdorder[nbeta][nalpha], computed_table_data[nbeta][nalpha] );
    }
  }
}

//----------------------------------------------------------------

/*
   The following gold values were obtained using a helper R-script from Brian Williams (LANL):

       # Specify P, c, and r
       P <- 0.95 # population coverage
       c <- 0.95 # confidence level
       
       r <- 1:20 # order statistic
       
       # one-sided
       
       oss <- numeric()
       for (ii in 1:length(r)) {
         oss[ii] <- r[ii]
         while (pbeta(1-P,r[ii],oss[ii]-r[ii]+1) < c) { oss[ii] <- oss[ii]+1 }
         print(oss)
       }
       
       # two-sided
       
       tss <- numeric()
       for (ii in 1:length(r)) {
         tss[ii] <- 2*r[ii]
         while (pbeta(P,tss[ii]-2*r[ii]+1,2*r[ii]) > 1-c) { tss[ii] <- tss[ii]+1 }
         print(tss)
       }

   The above also represents the implementation used in NonDSampling::compute_wilks_sample_size(...);
       
*/

TEUCHOS_UNIT_TEST(wilks, twosided_secondorder)
{
  std::map<int,int> gold_95_95,                    gold_99_999;
                    gold_95_95[1]  = 93  ;         gold_99_999[1]  = 920 ;
                    gold_95_95[2]  = 153 ;         gold_99_999[2]  = 1302;  
                    gold_95_95[3]  = 208 ;         gold_99_999[3]  = 1640;  
                    gold_95_95[4]  = 260 ;         gold_99_999[4]  = 1957;  
                    gold_95_95[5]  = 311 ;         gold_99_999[5]  = 2259;  
                    gold_95_95[6]  = 361 ;         gold_99_999[6]  = 2552;  
                    gold_95_95[7]  = 410 ;         gold_99_999[7]  = 2837;  
                    gold_95_95[8]  = 458 ;         gold_99_999[8]  = 3117;  
                    gold_95_95[9]  = 506 ;         gold_99_999[9]  = 3391;  
                    gold_95_95[10] = 554 ;         gold_99_999[10] = 3662;  
                    gold_95_95[11] = 601 ;         gold_99_999[11] = 3929;  
                    gold_95_95[12] = 647 ;         gold_99_999[12] = 4193;  
                    gold_95_95[13] = 694 ;         gold_99_999[13] = 4454;  
                    gold_95_95[14] = 740 ;         gold_99_999[14] = 4713;  
                    gold_95_95[15] = 786 ;         gold_99_999[15] = 4970;  
                    gold_95_95[16] = 832 ;         gold_99_999[16] = 5226;  
                    gold_95_95[17] = 877 ;         gold_99_999[17] = 5479;  
                    gold_95_95[18] = 923 ;         gold_99_999[18] = 5731;  
                    gold_95_95[19] = 968 ;         gold_99_999[19] = 5982;  
                    gold_95_95[20] = 1013;         gold_99_999[20] = 6231;

  
  // Test a commun use case
  Real alpha = 0.95, beta = 0.95;
  bool twosided = true;
  std::map<int,int>::const_iterator miter = gold_95_95.begin(),
                                     mend = gold_95_95.end();
  for( ; mend != miter; ++miter ) {
    int order = miter->first;
    int num_samples = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);
    TEST_EQUALITY( miter->second, num_samples );
  }


  // Test an extreme (costly?) use case
  alpha = 0.99;
  beta  = 0.999;
  miter = gold_99_999.begin();
  mend = gold_99_999.end();
  for( ; mend != miter; ++miter ) {
    int order = miter->first;
    int num_samples = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);
    TEST_EQUALITY( miter->second, num_samples );
  }
}


//----------------------------------------------------------------

/*
   The following tests exercise some Wilks utilities which compute a
   Wilks parameter given specified values for the other Wilks data.
*/

namespace {
  void plot_residuals(unsigned short order, int nsamples, Real alpha, Real beta, bool twosided)
  {
    std::ofstream ofile1("plot_alpha_resids.txt");
    std::ofstream ofile2("plot_beta_resids.txt");
    for( int i=0; i<101; ++i )
    {
      Real test_val = (Real)i/100.0;
      ofile1 << (Real)i/100.0 << "\t" << NonDSampling::compute_wilks_residual(order, nsamples, test_val, beta, twosided) << std::endl;
      ofile2 << (Real)i/100.0 << "\t" << NonDSampling::compute_wilks_residual(order, nsamples, alpha, test_val, twosided) << std::endl;
    }
    ofile1.close();
    ofile2.close();
  }
}


TEUCHOS_UNIT_TEST(wilks, compute_residual)
{
  Real alpha = 0.95, beta = 0.95;
  bool twosided = false;
  unsigned short order = 1;
  int num_samples = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);

  // Check agreement to within an integer
  Real res = 1.0 + NonDSampling::compute_wilks_residual(order, num_samples, alpha, beta, twosided);
  TEST_FLOATING_EQUALITY( 1.0, res, 1.0 );

  // Check agreement to within an integer
  order = 3;
  num_samples = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);
  res = 1.0 + NonDSampling::compute_wilks_residual(order, num_samples, alpha, beta, twosided);
  TEST_FLOATING_EQUALITY( 1.0, res, 1.0 );

  // Check agreement to within an integer
  twosided = true;
  num_samples = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);
  res = 1.0 + NonDSampling::compute_wilks_residual(order, num_samples, alpha, beta, twosided);
  TEST_FLOATING_EQUALITY( 1.0, res, 1.0 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(wilks, compute_alpha)
{
  const Real alpha = 0.97, beta = 0.99, max_tol = 4.e-4;

  bool twosided = false;
  unsigned short order = 1;
  int num_samples = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);
  Real test_alpha = NonDSampling::compute_wilks_alpha(order, num_samples, beta, twosided);
  Real test_alpha_m1 = NonDSampling::compute_wilks_alpha(order, num_samples-1, beta, twosided);
  Real tol = std::fabs(test_alpha - test_alpha_m1);
  TEST_ASSERT( tol < max_tol );
  TEST_FLOATING_EQUALITY( alpha, test_alpha, tol );

  twosided = false;
  order = 3;
  num_samples = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);
  test_alpha = NonDSampling::compute_wilks_alpha(order, num_samples, beta, twosided);
  test_alpha_m1 = NonDSampling::compute_wilks_alpha(order, num_samples-1, beta, twosided);
  tol = std::fabs(test_alpha - test_alpha_m1);
  TEST_ASSERT( tol < max_tol );
  TEST_FLOATING_EQUALITY( alpha, test_alpha, tol );

  twosided = true;
  order = 2;
  num_samples = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);
  test_alpha = NonDSampling::compute_wilks_alpha(order, num_samples, beta, twosided);
  test_alpha_m1 = NonDSampling::compute_wilks_alpha(order, num_samples-1, beta, twosided);
  tol = std::fabs(test_alpha - test_alpha_m1);
  TEST_ASSERT( tol < max_tol );
  TEST_FLOATING_EQUALITY( alpha, test_alpha, tol );

}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(wilks, compute_beta)
{
  const Real alpha = 0.97, beta = 0.99, max_tol = 4.e-4;

  bool twosided = false;
  unsigned short order = 1;
  int num_samples = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);
  Real test_beta = NonDSampling::compute_wilks_beta(order, num_samples, alpha, twosided);
  Real test_beta_m1 = NonDSampling::compute_wilks_beta(order, num_samples-1, alpha, twosided);
  Real tol = std::fabs(test_beta - test_beta_m1);
  TEST_ASSERT( tol < max_tol );
  TEST_FLOATING_EQUALITY( beta, test_beta, tol );

  twosided = false;
  order = 3;
  num_samples = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);
  test_beta = NonDSampling::compute_wilks_beta(order, num_samples, alpha, twosided);
  test_beta_m1 = NonDSampling::compute_wilks_beta(order, num_samples-1, alpha, twosided);
  tol = std::fabs(test_beta - test_beta_m1);
  TEST_ASSERT( tol < max_tol );
  TEST_FLOATING_EQUALITY( beta, test_beta, tol );

  twosided = true;
  order = 2;
  num_samples = NonDSampling::compute_wilks_sample_size(order, alpha, beta, twosided);
  test_beta = NonDSampling::compute_wilks_beta(order, num_samples, alpha, twosided);
  test_beta_m1 = NonDSampling::compute_wilks_beta(order, num_samples-1, alpha, twosided);
  tol = std::fabs(test_beta - test_beta_m1);
  TEST_ASSERT( tol < max_tol );
  TEST_FLOATING_EQUALITY( beta, test_beta, tol );
}
