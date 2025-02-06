/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef TOLERANCE_INTERVALS_H
#define TOLERANCE_INTERVALS_H

#include "dakota_data_types.hpp"

namespace Dakota {

/**
 *  \brief Given a required coverage c \in [0,1], this routine computes the value b such that
 *  
 *         F(b) = \int_{-b}^{b} (1/sqrt(2\pi)) * exp{-x*x/2) dx = c
 *
 *         There are two ways to compute b:
 *         1) b is the value such that \int_{-\infty}^{b} (1/sqrt(2\pi)) * exp{-x*x/2) dx = (1+c)/2.
 *            This is the approach used when calling the python scipy stats.norm.ppf((1+c)/2) routine.
 *         2) c = F(b) = 2 * \int_{0}^{b} (1/sqrt(2\pi)) * exp{-x*x/2) dx = erf(b/sqrt(2)), that is,
 *            b = sqrt(2) * erf_inv(c). This is the approach used in this DAKOTA routine.
 *
 *         If c \in [0,1], this routine returns b, otherwise it aborts/throws an exception.
 *         
 *  \param[in] coverage = the required coverage level c above
 *
 *  return = the value b explained above
 */
Real std_normal_coverage_inverse( const Real coverage );

/**
 *  \brief This routine computes the multiplicative conversion factor to be applied to the
 *         sample standard deviation in order to get the standard deviation corresponding
 *         to the 'two sided tolerance interval equivalent normal' (DSTIEN).
 *
 *         More especifically, this routine expects the following input information:
 *           - m, the number of samples;
 *           - alpha, a value in the closed interval [0,1] such that (1-alpha) is the required
 *             confidence level.
 *         
 *         The input information must satisty the following conditions, otherwise the routine
 *         aborts/throws an exception:
 *           - m >= 2;
 *           - alpha \in [0,1].
 *         
 *         If all input information is valid, then this routine returns the following value
 *         'mcf' for the multiplicative conversion factor:
 *           - mcf = sqrt(1. + 1./m)
 *                 * sqrt( (m - 1.) / quant )
 *                 * sqrt( 1. + (m - 3. - quant) / (2.*(m+1.)*(m+1.)) ),
 *         where:
 *           - quant is the alpha-quantile of a chi-square random variable with m-1 degrees
 *             of freedom.
 *
 *         See: Charles F. Jekel and Vicente Romero, "Conservative and Efficient Tail
 *              Probability Estimation from Sparse Sample Data", Sandia Report SAND2020-7572J,
 *              equations (4) and (5).
 *
 *  \param[in] number_of_samples = the value 'm' on the text above
 *  \param[in] alpha             = the value such that (1-alpha) is the required confidence level
 *
 *  return = value of the multiplicative conversion factor
 */
Real computeDSTIEN_conversion_factor( const size_t number_of_samples
                                    , const Real   alpha
                                    );

/**
 *  \brief This routine computes the r averages and r standard deviations corresponding to the
 *         'two sided tolerance interval equivalent normal' (DSTIEN).
 *
 *         More especifically, this routine expects the following input information:
 *           - n >= 2 response samples, each sample being a vector of r >= 1 responses;
 *           - c, the required coverage level;
 *           - a value alpha in the closed interval [0,1] such that (1-alpha) is the required
 *             confidence level.
 *         
 *         The input information must satisty the following conditions, otherwise the routine
 *         aborts/throws an exception:
 *           - n >= 2;
 *           - r >= 1, where r is the number of responses in the first response sample;
 *           - all remaining (n-1) response samples must have the same number r of resposens;
 *           - c \in [0,1];
 *           - alpha \in [0,1];
 *
 *         The output vectors dstien_mus and dstien_sigmas will have size r on output (that is,
 *         they are internally resized if they are not supplied with size r)
 *
 *         If all input information is valid, then this routine selects the m <= n response
 *         samples that are valid (meaning, all r responses are finite values):
 *           - if m == 0, this routine returns:
 *             - all r averages equal to std::numeric_limits<Real>::quiet_NaN(),
 *             - the delta_mf equal to std::numeric_limits<Real>::quiet_NaN(),
 *             - all r sample std deviations equal to std::numeric_limits<Real>::quiet_NaN(),
 *             - all r DSTIEN std deviations equal to std::numeric_limits<Real>::quiet_NaN();
 *           - if m == 1, this routine returns:
 *             - the r averages equal to the only valid sample,
 *             - the delta_mf equal to std::numeric_limits<Real>::quiet_NaN(),
 *             - all r sample std deviations equal to std::numeric_limits<Real>::quiet_NaN(),
 *             - all r DSTIEN std deviations equal to std::numeric_limits<Real>::quiet_NaN();
 *           - if m >= 2, this routine uses such m response samples to compute:
 *             - the r averages dstien_mus,
 *             - delta_mf = computeDSTIEN_conversion_factor(m, alpha)
 *                        * std_normal_coverage_inverse(c),
 *               so that the tolerance intervals are given by
 *               [dstien_mus - delta_mf*sample_sigmas, dstien_mus + delta_mf*sample_sigmas] 
 *             - the r sample standard deviations sample_sigmas,
 *             - the r DSTIEN standard deviations dstien_sigmas = 
 *               computeDSTIEN_conversion_factor(m, alpha) * sample_sigmas.
 *
 *  \param[in] resp_samples = the set of n response samples, each sample with r components
 *  \param[in] coverage     = the required coverage level c above
 *  \param[in] alpha        = the value such that (1-alpha) is the required confidence level
 *
 *  \param[out] num_valid_samples = number of valid samples ('m' on the text above) used to
 *                                  compute 'ties_mus' and 'dstien_sigmas'
 *  \param[out] dstien_mus        = the r averages
 *  \param[out] delta_mf          = the multiplicative factor explained above
 *  \param[out] sample_sigmas     = the r sample standard deviations
 *  \param[out] dstien_sigmas     = the r DSTIEN standard deviations
 */
void computeDSTIEN( const IntResponseMap & resp_samples
                  , const Real             coverage
                  , const Real             alpha
                  , size_t               & num_valid_samples
                  , RealVector           & dstien_mus
                  , Real                 & delta_mf
                  , RealVector           & sample_sigmas
                  , RealVector           & dstien_sigmas
                  );

} // namespace Dakota

#endif
