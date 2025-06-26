/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <gtest/gtest.h>

#include <boost/math/distributions/chi_squared.hpp>
#include <cassert>
#include <iostream>

#include "DakotaResponse.hpp"
#include "tolerance_intervals.hpp"

namespace Dakota {
namespace TestToleranceIntervals {

// *************************************************************************
// Tests for the std_normal_coverage_inverse() routine
// *************************************************************************
void test_snci_valid_input_01_benchmark() {
  // Reference python program:
  //
  // from scipy import stats
  // print( stats.norm.ppf((1+0.80)/2.0) )
  // print( stats.norm.ppf((1+0.85)/2.0) )
  // print( stats.norm.ppf((1+0.90)/2.0) )
  // print( stats.norm.ppf((1+0.95)/2.0) )
  //
  // Output on the screen is:
  // 1.2815515655446004
  // 1.4395314709384563
  // 1.6448536269514722
  // 1.959963984540054

  EXPECT_TRUE(
      (std::abs(std_normal_coverage_inverse(0.80) - 1.2815515655446004) <
       10. * std::numeric_limits<Real>::epsilon()));
  EXPECT_TRUE(
      (std::abs(std_normal_coverage_inverse(0.85) - 1.4395314709384563) <
       10. * std::numeric_limits<Real>::epsilon()));
  EXPECT_TRUE(
      (std::abs(std_normal_coverage_inverse(0.90) - 1.6448536269514722) <
       10. * std::numeric_limits<Real>::epsilon()));
  EXPECT_TRUE((std::abs(std_normal_coverage_inverse(0.95) - 1.959963984540054) <
               10. * std::numeric_limits<Real>::epsilon()));
}

void test_snci_invalid_input_01_coverageOutOfRange() {
  EXPECT_THROW(std_normal_coverage_inverse(-0.05), std::system_error);

  EXPECT_THROW(std_normal_coverage_inverse(-1.05), std::system_error);
}

// *************************************************************************
// Tests for the computeDSTIEN_conversion_factor() routine
// *************************************************************************
void test_DSTIEN_mcf_valid_input_01_benchmark() {
  {
    Real m = 5;
    Real alpha = 0.05;
    Real mcf = computeDSTIEN_conversion_factor(m, alpha);

    boost::math::chi_squared chisq(m - 1.);
    Real quant = boost::math::quantile(chisq, alpha);
    Real referenceMCF =
        sqrt(1. + 1. / m) * sqrt((m - 1.) / quant) *
        sqrt(1. + (m - 3. - quant) / (2. * (m + 1.) * (m + 1.)));

    // std::cout << "In test_DSTIEN_mcf_valid_input_01_benchmark()"
    //           << ": mcf = "          << mcf
    //           << ", referenceMCF = " << referenceMCF
    //           << ", std::numeric_limits<Real>::epsilon() = " <<
    //           std::numeric_limits<Real>::epsilon()
    //           << std::endl;

    EXPECT_TRUE((std::abs(mcf - referenceMCF) <
                 10. * std::numeric_limits<Real>::epsilon()));
  }

  {
    Real m = 17;
    Real alpha = 0.09;
    Real mcf = computeDSTIEN_conversion_factor(m, alpha);

    boost::math::chi_squared chisq(m - 1.);
    Real quant = boost::math::quantile(chisq, alpha);
    Real referenceMCF =
        sqrt(1. + 1. / m) * sqrt((m - 1.) / quant) *
        sqrt(1. + (m - 3. - quant) / (2. * (m + 1.) * (m + 1.)));

    // std::cout << "In test_DSTIEN_mcf_valid_input_01_benchmark()"
    //           << ": mcf = "          << mcf
    //           << ", referenceMCF = " << referenceMCF
    //           << ", std::numeric_limits<Real>::epsilon() = " <<
    //           std::numeric_limits<Real>::epsilon()
    //           << std::endl;

    EXPECT_TRUE((std::abs(mcf - referenceMCF) <
                 10. * std::numeric_limits<Real>::epsilon()));
  }

  {
    Real mcf = computeDSTIEN_conversion_factor(5, 0.1);
    Real referenceMCF = 2.138122377564248;  // Value computed by python routine

    std::cout << "In test_DSTIEN_mcf_valid_input_01_benchmark()"
              << ": mcf = " << std::setprecision(16) << mcf
              << ", referenceMCF = " << std::setprecision(16) << referenceMCF
              << ", std::numeric_limits<Real>::epsilon() = "
              << std::numeric_limits<Real>::epsilon() << std::endl;

    EXPECT_TRUE((std::abs(mcf - referenceMCF) <
                 10. * std::numeric_limits<Real>::epsilon()));
  }
}

void test_DSTIEN_mcf_valid_input_02_regression() {
  Real m = 17;
  Real alpha = 0.06;
  Real mcf = computeDSTIEN_conversion_factor(m, alpha);

  Real referenceMCF = 1.436456320254741;

  std::cout << "In test_DSTIEN_mcf_valid_input_01_regression()"
            << ": mcf = " << std::setprecision(16) << mcf
            << ", referenceMCF = " << std::setprecision(16) << referenceMCF
            << ", std::numeric_limits<Real>::epsilon() = "
            << std::numeric_limits<Real>::epsilon() << std::endl;

  EXPECT_TRUE((std::abs(mcf - referenceMCF) <
               10. * std::numeric_limits<Real>::epsilon()));
}

void test_DSTIEN_mcf_invalid_input_01_noSamples() {
  EXPECT_THROW(computeDSTIEN_conversion_factor(0, 0.05  // alpha
                                               ),
               std::system_error);
}

void test_DSTIEN_mcf_invalid_input_02_justOneSample() {
  EXPECT_THROW(computeDSTIEN_conversion_factor(1, 0.05  // alpha
                                               ),
               std::system_error);
}

void test_DSTIEN_mcf_invalid_input_03_alphaOutOfRange() {
  EXPECT_THROW(computeDSTIEN_conversion_factor(2, -0.05  // alpha
                                               ),
               std::system_error);
  EXPECT_THROW(computeDSTIEN_conversion_factor(2, 1.05  // alpha
                                               ),
               std::system_error);
}

// *************************************************************************
// Tests for the computeDSTIEN() routine
// *************************************************************************
void test_DSTIEN_valid_input_01_benchmark() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 3;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);
  respA.function_value_view(0) = 13.2;
  respA.function_value_view(1) = -13.8;
  respA.function_value_view(2) = 0.104;

  Dakota::SharedResponseData srdB(as);
  Response respB(srdB);
  respB.function_value_view(0) = 13.4;
  respB.function_value_view(1) = -13.4;
  respB.function_value_view(2) = 0.114;

  Dakota::SharedResponseData srdC(as);
  Response respC(srdC);
  respC.function_value_view(0) = 13.6;
  respC.function_value_view(1) = -13.0;
  respC.function_value_view(2) = 0.124;

  Dakota::SharedResponseData srdD(as);
  Response respD(srdD);
  respD.function_value_view(0) = 13.8;
  respD.function_value_view(1) = -12.6;
  respD.function_value_view(2) = 0.134;

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));
  resp_samples.insert(std::pair<int, Response>(1, respB));
  resp_samples.insert(std::pair<int, Response>(2, respC));
  resp_samples.insert(std::pair<int, Response>(3, respD));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);
  computed_dstien_mus[0] = 888.1;
  computed_dstien_mus[1] = 777.3;
  computed_dstien_mus[2] = 999.2;

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(num_fns);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);
  computed_dstien_sigmas[0] = -3888.1;
  computed_dstien_sigmas[1] = -1777.3;
  computed_dstien_sigmas[2] = -2999.2;

  Real coverage = 0.85;
  Real alpha = 0.05;
  computeDSTIEN(resp_samples, coverage, alpha, num_valid_samples,
                computed_dstien_mus, computed_multiplicative_factor,
                computed_sample_sigmas, computed_dstien_sigmas);

  // std::cout << "In test_DSTIEN_valid_input_01_benchmark()"
  //           << ": computed_dstien_mus = "    << computed_dstien_mus
  //           << ", computed_dstien_sigmas = " << computed_dstien_sigmas
  //           << ", std::numeric_limits<Real>::epsilon() = " <<
  //           std::numeric_limits<Real>::epsilon()
  //           << std::endl;

  // ************************************************************************
  // Check the results
  // ************************************************************************
  std::vector<Real> reference_dstien_mus(num_fns);
  reference_dstien_mus[0] = 13.5;
  reference_dstien_mus[1] = -13.2;
  reference_dstien_mus[2] = 0.119;

  Real m = 4;
  Real mcf = computeDSTIEN_conversion_factor(m, alpha);

  std::vector<Real> reference_dstien_sigmas(num_fns);
  reference_dstien_sigmas[0] =
      mcf * sqrt(0.3 * 0.3 + 0.1 * 0.1 + 0.1 * 0.1 + 0.3 * 0.3) / sqrt(m - 1.);
  reference_dstien_sigmas[1] =
      mcf * sqrt(0.6 * 0.6 + 0.2 * 0.2 + 0.2 * 0.2 + 0.6 * 0.6) / sqrt(m - 1.);
  reference_dstien_sigmas[2] =
      mcf *
      sqrt(0.015 * 0.015 + 0.005 * 0.005 + 0.005 * 0.005 + 0.015 * 0.015) /
      sqrt(m - 1.);

  EXPECT_TRUE((num_valid_samples == 4));
  for (size_t k = 0; k < num_fns; ++k) {
    EXPECT_TRUE((std::abs(computed_dstien_mus[k] - reference_dstien_mus[k]) <
                 10. * std::numeric_limits<Real>::epsilon()));
    EXPECT_TRUE(
        (std::abs(computed_dstien_sigmas[k] - reference_dstien_sigmas[k]) <
         10. * std::numeric_limits<Real>::epsilon()));
  }
}

void test_DSTIEN_valid_input_02_regression() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 3;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);
  respA.function_value_view(0) = 13.2;
  respA.function_value_view(1) = -13.8;
  respA.function_value_view(2) = 0.104;

  Dakota::SharedResponseData srdB(as);
  Response respB(srdB);
  respB.function_value_view(0) = 10.4;
  respB.function_value_view(1) = -13.2;
  respB.function_value_view(2) = 0.138;

  Dakota::SharedResponseData srdC(as);
  Response respC(srdC);
  respC.function_value_view(0) = 13.8;
  respC.function_value_view(1) = -10.4;
  respC.function_value_view(2) = 0.132;

  Dakota::SharedResponseData srdD(as);
  Response respD(srdD);
  respD.function_value_view(0) = 14.1;
  respD.function_value_view(1) = -14.3;
  respD.function_value_view(2) = 0.49;

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));
  resp_samples.insert(std::pair<int, Response>(1, respB));
  resp_samples.insert(std::pair<int, Response>(2, respC));
  resp_samples.insert(std::pair<int, Response>(3, respD));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);
  computed_dstien_mus[0] = 888.1;
  computed_dstien_mus[1] = 777.3;
  computed_dstien_mus[2] = 999.2;

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(num_fns);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);
  computed_dstien_sigmas[0] = -3888.1;
  computed_dstien_sigmas[1] = -1777.3;
  computed_dstien_sigmas[2] = -2999.2;

  computeDSTIEN(resp_samples, 0.85  // coverage
                ,
                0.05  // alpha
                ,
                num_valid_samples, computed_dstien_mus,
                computed_multiplicative_factor, computed_sample_sigmas,
                computed_dstien_sigmas);

  // std::cout << "In test_DSTIEN_valid_input_02_regression()"
  //           << ": computed_dstien_mus = "    << std::setprecision(16) <<
  //           computed_dstien_mus
  //           << ", computed_dstien_sigmas = " << std::setprecision(16) <<
  //           computed_dstien_sigmas
  //           << ", std::numeric_limits<Real>::epsilon() = " <<
  //           std::numeric_limits<Real>::epsilon()
  //           << std::endl;

  // ************************************************************************
  // Check the results
  // ************************************************************************
  std::vector<Real> reference_dstien_mus(num_fns);
  reference_dstien_mus[0] = 1.2875000000e+01;
  reference_dstien_mus[1] = -1.2925000000e+01;
  reference_dstien_mus[2] = 2.1600000000e-01;

  std::vector<Real> reference_dstien_sigmas(num_fns);
  reference_dstien_sigmas[0] = 5.5591537226e+00;
  reference_dstien_sigmas[1] = 5.7249921868e+00;
  reference_dstien_sigmas[2] = 6.0217037786e-01;

  EXPECT_TRUE((num_valid_samples == 4));
  for (size_t k = 0; k < num_fns; ++k) {
    EXPECT_TRUE((std::abs(computed_dstien_mus[k] - reference_dstien_mus[k]) <
                 10. * std::numeric_limits<Real>::epsilon()));
    EXPECT_TRUE(
        (std::abs(computed_dstien_sigmas[k] - reference_dstien_sigmas[k]) <
         1000000. * std::numeric_limits<Real>::epsilon()));
  }
}

void test_DSTIEN_valid_input_03_allResponsesEqual() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 3;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);
  respA.function_value_view(0) = 13.2;
  respA.function_value_view(1) = -13.8;
  respA.function_value_view(2) = 0.104;

  Dakota::SharedResponseData srdB(as);
  Response respB(srdB);
  respB.function_value_view(0) = 13.2;
  respB.function_value_view(1) = -13.8;
  respB.function_value_view(2) = 0.104;

  Dakota::SharedResponseData srdC(as);
  Response respC(srdC);
  respC.function_value_view(0) = 13.2;
  respC.function_value_view(1) = -13.8;
  respC.function_value_view(2) = 0.104;

  Dakota::SharedResponseData srdD(as);
  Response respD(srdD);
  respD.function_value_view(0) = 13.2;
  respD.function_value_view(1) = -13.8;
  respD.function_value_view(2) = 0.104;

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));
  resp_samples.insert(std::pair<int, Response>(1, respB));
  resp_samples.insert(std::pair<int, Response>(2, respC));
  resp_samples.insert(std::pair<int, Response>(3, respD));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);
  computed_dstien_mus[0] = 888.1;
  computed_dstien_mus[1] = 777.3;
  computed_dstien_mus[2] = 999.2;

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(num_fns);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);
  computed_dstien_sigmas[0] = -3888.1;
  computed_dstien_sigmas[1] = -1777.3;
  computed_dstien_sigmas[2] = -2999.2;

  computeDSTIEN(resp_samples, 0.85  // coverage
                ,
                0.05  // alpha
                ,
                num_valid_samples, computed_dstien_mus,
                computed_multiplicative_factor, computed_sample_sigmas,
                computed_dstien_sigmas);

  // std::cout << "In test_DSTIEN_valid_input_03_allResponsesEqual()"
  //           << ": computed_dstien_mus = "    << computed_dstien_mus
  //           << ", computed_dstien_sigmas = " << computed_dstien_sigmas
  //           << ", std::numeric_limits<Real>::epsilon() = " <<
  //           std::numeric_limits<Real>::epsilon()
  //           << std::endl;

  // ************************************************************************
  // Check the results
  // ************************************************************************
  std::vector<Real> expected_dstien_mus(num_fns);
  expected_dstien_mus[0] = 13.2;
  expected_dstien_mus[1] = -13.8;
  expected_dstien_mus[2] = 0.104;

  std::vector<Real> expected_dstien_sigmas(num_fns);
  expected_dstien_sigmas[0] = 0.;
  expected_dstien_sigmas[1] = 0.;
  expected_dstien_sigmas[2] = 0.;

  EXPECT_TRUE((num_valid_samples == 4));
  for (size_t k = 0; k < num_fns; ++k) {
    EXPECT_TRUE((computed_dstien_mus[k] == expected_dstien_mus[k]));
    EXPECT_TRUE((computed_dstien_sigmas[k] == expected_dstien_sigmas[k]));
  }
}

void test_DSTIEN_valid_input_04_noValidResponseSamples() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 3;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);
  respA.function_value_view(0) = 13.2;
  respA.function_value_view(1) = -13.8;
  respA.function_value_view(2) = std::numeric_limits<Real>::quiet_NaN();

  Dakota::SharedResponseData srdB(as);
  Response respB(srdB);
  respB.function_value_view(0) = 10.4;
  respB.function_value_view(1) = std::numeric_limits<Real>::quiet_NaN();
  respB.function_value_view(2) = 0.138;

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));
  resp_samples.insert(std::pair<int, Response>(1, respB));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);
  computed_dstien_mus[0] = 888.1;
  computed_dstien_mus[1] = 777.3;
  computed_dstien_mus[2] = 999.2;

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(num_fns);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);
  computed_dstien_sigmas[0] = -3888.1;
  computed_dstien_sigmas[1] = -1777.3;
  computed_dstien_sigmas[2] = -2999.2;

  computeDSTIEN(resp_samples, 0.85  // coverage
                ,
                0.05  // alpha
                ,
                num_valid_samples, computed_dstien_mus,
                computed_multiplicative_factor, computed_sample_sigmas,
                computed_dstien_sigmas);

  // ************************************************************************
  // Check the results
  // ************************************************************************
  EXPECT_TRUE((num_valid_samples == 0));
  for (size_t k = 0; k < num_fns; ++k) {
    EXPECT_TRUE((std::isnan(computed_dstien_mus[k])));
    EXPECT_TRUE((std::isnan(computed_dstien_sigmas[k])));
  }
}

void test_DSTIEN_valid_input_05_justOneValidResponseSample() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 3;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);
  respA.function_value_view(0) = 13.2;
  respA.function_value_view(1) = -13.8;
  respA.function_value_view(2) = 0.104;

  Dakota::SharedResponseData srdB(as);
  Response respB(srdB);
  respB.function_value_view(0) = 10.4;
  respB.function_value_view(1) = std::numeric_limits<Real>::quiet_NaN();
  respB.function_value_view(2) = 0.138;

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));
  resp_samples.insert(std::pair<int, Response>(1, respB));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);
  computed_dstien_mus[0] = 888.1;
  computed_dstien_mus[1] = 777.3;
  computed_dstien_mus[2] = 999.2;

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(num_fns);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);
  computed_dstien_sigmas[0] = -3888.1;
  computed_dstien_sigmas[1] = -1777.3;
  computed_dstien_sigmas[2] = -2999.2;

  computeDSTIEN(resp_samples, 0.85  // coverage
                ,
                0.05  // alpha
                ,
                num_valid_samples, computed_dstien_mus,
                computed_multiplicative_factor, computed_sample_sigmas,
                computed_dstien_sigmas);

  // ************************************************************************
  // Check the results
  // ************************************************************************
  std::vector<Real> expected_dstien_mus(num_fns);
  expected_dstien_mus[0] = 13.2;
  expected_dstien_mus[1] = -13.8;
  expected_dstien_mus[2] = 0.104;

  EXPECT_TRUE((num_valid_samples == 1));
  for (size_t k = 0; k < num_fns; ++k) {
    EXPECT_TRUE((computed_dstien_mus[k] == expected_dstien_mus[k]));
    EXPECT_TRUE((std::isnan(computed_dstien_sigmas[k])));
  }
}

void test_DSTIEN_valid_input_06_differentMuSize() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 2;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);
  respA.function_value_view(0) = 10.;
  respA.function_value_view(1) = -10.;

  Dakota::SharedResponseData srdB(as);
  Response respB(srdB);
  respB.function_value_view(0) = 12.;
  respB.function_value_view(1) = -12.;

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));
  resp_samples.insert(std::pair<int, Response>(1, respB));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(1);

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(1);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);

  Real coverage = 0.85;
  Real alpha = 0.05;
  computeDSTIEN(resp_samples, coverage, alpha, num_valid_samples,
                computed_dstien_mus, computed_multiplicative_factor,
                computed_sample_sigmas, computed_dstien_sigmas);

  // ************************************************************************
  // Check the results
  // ************************************************************************
  std::vector<Real> expected_dstien_mus(num_fns);
  expected_dstien_mus[0] = 11.;
  expected_dstien_mus[1] = -11.;

  Real m = 2;
  Real mcf = computeDSTIEN_conversion_factor(m, alpha);

  std::vector<Real> expected_dstien_sigmas(num_fns);
  expected_dstien_sigmas[0] = mcf * sqrt(1. * 1. + 1. * 1.) / sqrt(m - 1.);
  expected_dstien_sigmas[1] = mcf * sqrt(1. * 1. + 1. * 1.) / sqrt(m - 1.);

  EXPECT_TRUE((num_valid_samples == 2));
  EXPECT_TRUE((computed_dstien_mus.length() == num_fns));
  for (size_t k = 0; k < num_fns; ++k) {
    EXPECT_TRUE((computed_dstien_mus[k] == expected_dstien_mus[k]));
    EXPECT_TRUE((computed_dstien_sigmas[k] == expected_dstien_sigmas[k]));
  }
}

void test_DSTIEN_valid_input_07_differentSigmaSize() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 2;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);
  respA.function_value_view(0) = 10.;
  respA.function_value_view(1) = -10.;

  Dakota::SharedResponseData srdB(as);
  Response respB(srdB);
  respB.function_value_view(0) = 12.;
  respB.function_value_view(1) = -12.;

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));
  resp_samples.insert(std::pair<int, Response>(1, respB));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(1);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(3);

  Real coverage = 0.85;
  Real alpha = 0.05;
  computeDSTIEN(resp_samples, coverage, alpha, num_valid_samples,
                computed_dstien_mus, computed_multiplicative_factor,
                computed_sample_sigmas, computed_dstien_sigmas);

  // ************************************************************************
  // Check the results
  // ************************************************************************
  std::vector<Real> expected_dstien_mus(num_fns);
  expected_dstien_mus[0] = 11.;
  expected_dstien_mus[1] = -11.;

  Real m = 2;
  Real mcf = computeDSTIEN_conversion_factor(m, alpha);

  std::vector<Real> expected_dstien_sigmas(num_fns);
  expected_dstien_sigmas[0] = mcf * sqrt(1. * 1. + 1. * 1.) / sqrt(m - 1.);
  expected_dstien_sigmas[1] = mcf * sqrt(1. * 1. + 1. * 1.) / sqrt(m - 1.);

  EXPECT_TRUE((num_valid_samples == 2));
  EXPECT_TRUE((computed_dstien_sigmas.length() == num_fns));
  for (size_t k = 0; k < num_fns; ++k) {
    EXPECT_TRUE((computed_dstien_mus[k] == expected_dstien_mus[k]));
    EXPECT_TRUE((computed_dstien_sigmas[k] == expected_dstien_sigmas[k]));
  }
}

void test_DSTIEN_invalid_input_01_noResponseSamples() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 3;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  IntResponseMap resp_samples;

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(num_fns);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);

  EXPECT_THROW(computeDSTIEN(resp_samples, 0.85  // coverage
                             ,
                             0.05  // alpha
                             ,
                             num_valid_samples, computed_dstien_mus,
                             computed_multiplicative_factor,
                             computed_sample_sigmas, computed_dstien_sigmas),
               std::system_error);
}

void test_DSTIEN_invalid_input_02_justOneResponseSample() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 3;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);
  respA.function_value_view(0) = 13.2;
  respA.function_value_view(1) = -13.8;
  respA.function_value_view(2) = 0.104;

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(num_fns);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);

  EXPECT_THROW(computeDSTIEN(resp_samples, 0.85  // coverage
                             ,
                             0.05  // alpha
                             ,
                             num_valid_samples, computed_dstien_mus,
                             computed_multiplicative_factor,
                             computed_sample_sigmas, computed_dstien_sigmas),
               std::system_error);
}

void test_DSTIEN_invalid_input_03_noResponses() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 0;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);

  Dakota::SharedResponseData srdB(as);
  Response respB(srdB);

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));
  resp_samples.insert(std::pair<int, Response>(1, respB));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(num_fns);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);

  EXPECT_THROW(computeDSTIEN(resp_samples, 0.85  // coverage
                             ,
                             0.05  // alpha
                             ,
                             num_valid_samples, computed_dstien_mus,
                             computed_multiplicative_factor,
                             computed_sample_sigmas, computed_dstien_sigmas),
               std::system_error);
}

void test_DSTIEN_invalid_input_04_differentResponseSizes() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 3;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);

  Dakota::ActiveSet asEmpty(0, num_derivs);
  Dakota::SharedResponseData srdB(asEmpty);
  Response respB(srdB);

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));
  resp_samples.insert(std::pair<int, Response>(1, respB));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(num_fns);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);

  EXPECT_THROW(computeDSTIEN(resp_samples, 0.85  // coverage
                             ,
                             0.05  // alpha
                             ,
                             num_valid_samples, computed_dstien_mus,
                             computed_multiplicative_factor,
                             computed_sample_sigmas, computed_dstien_sigmas),
               std::system_error);
}

void test_DSTIEN_invalid_input_05_coverageOutOfRange() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 3;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);
  respA.function_value_view(0) = 13.2;
  respA.function_value_view(1) = -13.8;
  respA.function_value_view(2) = 0.104;

  Dakota::SharedResponseData srdB(as);
  Response respB(srdB);
  respB.function_value_view(0) = 13.2;
  respB.function_value_view(1) = -13.8;
  respB.function_value_view(2) = 0.104;

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));
  resp_samples.insert(std::pair<int, Response>(1, respB));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(num_fns);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);

  EXPECT_THROW(computeDSTIEN(resp_samples, -0.01  // coverage
                             ,
                             0.85  // alpha
                             ,
                             num_valid_samples, computed_dstien_mus,
                             computed_multiplicative_factor,
                             computed_sample_sigmas, computed_dstien_sigmas),
               std::system_error);

  EXPECT_THROW(computeDSTIEN(resp_samples, 1.01  // coverage
                             ,
                             0.85  // alpha
                             ,
                             num_valid_samples, computed_dstien_mus,
                             computed_multiplicative_factor,
                             computed_sample_sigmas, computed_dstien_sigmas),
               std::system_error);
}

void test_DSTIEN_invalid_input_06_alphaOutOfRange() {
  // ************************************************************************
  // Generate response samples
  // ************************************************************************
  size_t num_fns = 3;
  size_t num_derivs = 0;
  Dakota::ActiveSet as(num_fns, num_derivs);

  Dakota::SharedResponseData srdA(as);
  Response respA(srdA);
  respA.function_value_view(0) = 13.2;
  respA.function_value_view(1) = -13.8;
  respA.function_value_view(2) = 0.104;

  Dakota::SharedResponseData srdB(as);
  Response respB(srdB);
  respB.function_value_view(0) = 13.2;
  respB.function_value_view(1) = -13.8;
  respB.function_value_view(2) = 0.104;

  IntResponseMap resp_samples;
  resp_samples.insert(std::pair<int, Response>(0, respA));
  resp_samples.insert(std::pair<int, Response>(1, respB));

  // ************************************************************************
  // Compute DSTIEN mus and DSTIEN sigmas
  // ************************************************************************
  size_t num_valid_samples = 999;

  RealVector computed_dstien_mus(num_fns);

  Real computed_multiplicative_factor(0.);
  RealVector computed_sample_sigmas(num_fns);
  computed_sample_sigmas = 0.;

  RealVector computed_dstien_sigmas(num_fns);

  EXPECT_THROW(computeDSTIEN(resp_samples, 0.85  // coverage
                             ,
                             -0.01  // alpha
                             ,
                             num_valid_samples, computed_dstien_mus,
                             computed_multiplicative_factor,
                             computed_sample_sigmas, computed_dstien_sigmas),
               std::system_error);

  EXPECT_THROW(computeDSTIEN(resp_samples, 0.85  // coverage
                             ,
                             1.01  // alpha
                             ,
                             num_valid_samples, computed_dstien_mus,
                             computed_multiplicative_factor,
                             computed_sample_sigmas, computed_dstien_sigmas),
               std::system_error);
}

}  // end namespace TestToleranceIntervals
}  // end namespace Dakota

//____________________________________________________________________________//

TEST(tolerance_intervals_tests, all_tests) {
  Dakota::abort_mode = Dakota::ABORT_THROWS;

  // *************************************************************************
  // Tests for the std_normal_coverage_inverse() routine
  // *************************************************************************
  Dakota::TestToleranceIntervals::test_snci_valid_input_01_benchmark();
  // std::cout << "test_snci_valid_input_01_benchmark() passed" << std::endl;

  Dakota::TestToleranceIntervals::
      test_snci_invalid_input_01_coverageOutOfRange();
  // std::cout << "test_snci_invalid_input_01_coverageOutOfRange() passed" <<
  // std::endl;

  // *************************************************************************
  // Tests for the computeDSTIEN_conversion_factor() routine
  // *************************************************************************
  Dakota::TestToleranceIntervals::test_DSTIEN_mcf_valid_input_01_benchmark();
  // std::cout << "test_DSTIEN_mcf_valid_input_01_benchmark() passed" <<
  // std::endl;

  Dakota::TestToleranceIntervals::test_DSTIEN_mcf_valid_input_02_regression();
  // std::cout << "test_DSTIEN_mcf_valid_input_02_regression() passed" <<
  // std::endl;

  Dakota::TestToleranceIntervals::test_DSTIEN_mcf_invalid_input_01_noSamples();
  // std::cout << "test_DSTIEN_mcf_invalid_input_01_noSamples() passed" <<
  // std::endl;

  Dakota::TestToleranceIntervals::
      test_DSTIEN_mcf_invalid_input_02_justOneSample();
  // std::cout << "test_DSTIEN_mcf_invalid_input_02_justOneSample() passed" <<
  // std::endl;

  Dakota::TestToleranceIntervals::
      test_DSTIEN_mcf_invalid_input_03_alphaOutOfRange();
  // std::cout << "test_DSTIEN_mcf_invalid_input_03_alphaOutOfRange() passed" <<
  // std::endl;

  // *************************************************************************
  // Tests for the computeDSTIEN() routine
  // *************************************************************************
  Dakota::TestToleranceIntervals::test_DSTIEN_valid_input_01_benchmark();
  // std::cout << "test_DSTIEN_valid_input_01_benchmark() passed" << std::endl;

  Dakota::TestToleranceIntervals::test_DSTIEN_valid_input_02_regression();
  // std::cout << "test_DSTIEN_valid_input_02_regression() passed" << std::endl;

  Dakota::TestToleranceIntervals::
      test_DSTIEN_valid_input_03_allResponsesEqual();
  // std::cout << "test_DSTIEN_valid_input_03_allResponsesEqual() passed" <<
  // std::endl;

  Dakota::TestToleranceIntervals::
      test_DSTIEN_valid_input_04_noValidResponseSamples();
  // std::cout << "test_DSTIEN_valid_input_04_noValidResponseSamples() passed"
  // << std::endl;

  Dakota::TestToleranceIntervals::
      test_DSTIEN_valid_input_05_justOneValidResponseSample();
  // std::cout << "test_DSTIEN_valid_input_05_justOneValidResponseSample()
  // passed" << std::endl;

  Dakota::TestToleranceIntervals::test_DSTIEN_valid_input_06_differentMuSize();
  // std::cout << "test_DSTIEN_invalid_input_07_differentMuSize() passed" <<
  // std::endl;

  Dakota::TestToleranceIntervals::
      test_DSTIEN_valid_input_07_differentSigmaSize();
  // std::cout << "test_DSTIEN_invalid_input_08_differentSigmaSize() passed" <<
  // std::endl;

  Dakota::TestToleranceIntervals::
      test_DSTIEN_invalid_input_01_noResponseSamples();
  // std::cout << "test_DSTIEN_invalid_input_01_noResponseSamples() passed" <<
  // std::endl;

  Dakota::TestToleranceIntervals::
      test_DSTIEN_invalid_input_02_justOneResponseSample();
  // std::cout << "test_DSTIEN_invalid_input_02_justOneResponseSample() passed"
  // << std::endl;

  Dakota::TestToleranceIntervals::test_DSTIEN_invalid_input_03_noResponses();
  // std::cout << "test_DSTIEN_invalid_input_03_noResponses() passed" <<
  // std::endl;

  Dakota::TestToleranceIntervals::
      test_DSTIEN_invalid_input_04_differentResponseSizes();
  // std::cout << "test_DSTIEN_invalid_input_04_differentResponseSizes() passed"
  // << std::endl;

  Dakota::TestToleranceIntervals::
      test_DSTIEN_invalid_input_05_coverageOutOfRange();
  // std::cout << "test_DSTIEN_invalid_input_05_coverageOutOfRange() passed" <<
  // std::endl;

  Dakota::TestToleranceIntervals::
      test_DSTIEN_invalid_input_06_alphaOutOfRange();
  // std::cout << "test_DSTIEN_invalid_input_06_alphaOutOfRange() passed" <<
  // std::endl;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
