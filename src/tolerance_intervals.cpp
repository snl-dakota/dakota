/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "tolerance_intervals.hpp"
#include "DakotaResponse.hpp"
#include <boost/math/distributions/chi_squared.hpp>

static const char rcsId[]="@(#) $Id: tolerance_intervals.cpp 9999 2010-10-22 23:20:24Z mseldre $";

namespace Dakota {

Real std_normal_coverage_inverse( const Real coverage )
{
  if ((0. <= coverage) && (coverage <= 1.)) {
    // Ok
  }
  else {
    Cerr << "Error in std_normal_coverage_inverse()"
         << ": coverage (" << coverage
         << ") must belong to the closed interval [0,1]"
         << std::endl;
    abort_handler(-1);
  }

  Real b = sqrt(2.) * boost::math::erf_inv(coverage);

  return b;
}

Real computeDSTIEN_conversion_factor( const size_t number_of_samples
                                    , const Real   alpha
                                    )
{
  if (number_of_samples < 2) {
    Cerr << "Error in computeDSTIEN_conversion_factor()"
         << ": the number of response samples (" << number_of_samples
         << ") must be at least 2"
         << std::endl;
    abort_handler(-1);
  }

  if ((0. <= alpha) && (alpha <= 1.)) {
    // Ok
  }
  else {
    Cerr << "Error in computeDSTIEN_conversion_factor()"
         << ": alpha (" << alpha
         << ") must belong to the closed interval [0,1]"
         << std::endl;
    abort_handler(-1);
  }

  Real conversionFactor = 0.;
  
  if (number_of_samples == 1) {
    conversionFactor = std::numeric_limits<Real>::quiet_NaN();
  }
  else {
    Real m = static_cast<Real>(number_of_samples);
    boost::math::chi_squared chisq(m - 1.);
    Real quant = boost::math::quantile(chisq, alpha);
    Real tmpRatio = (m - 3. - quant) / (2. * (m + 1.) * (m + 1.));
    conversionFactor = std::sqrt(1. + 1./m) * std::sqrt( (m - 1.)/quant ) * std::sqrt( 1. + tmpRatio );
  }

  return conversionFactor;
}

void computeDSTIEN( const IntResponseMap & resp_samples
                  , const Real             coverage
                  , const Real             alpha
                  , size_t               & num_valid_samples
                  , RealVector           & dstien_mus
                  , Real                 & delta_mf
                  , RealVector           & sample_sigmas
                  , RealVector           & dstien_sigmas
                  )
{
  // Check input information
  size_t num_samples = resp_samples.size();
  if (num_samples < 2) {
    Cerr << "Error in computeDSTIEN()"
         << ": the number of response samples (" << num_samples
         << ") must be at least 2"
         << std::endl;
    abort_handler(-1);
  }

  size_t num_responses = resp_samples.begin()->second.num_functions();
  if (num_responses == 0) {
    Cerr << "Error in computeDSTIEN()"
         << ": the number of responses of the first sample (" << num_responses
         << ") must be nonzero"
         << std::endl;
    abort_handler(-1);
  }

  for (IntRespMCIter it = resp_samples.begin(); it != resp_samples.end(); ++it) {
    if (it->second.num_functions() != num_responses) {
      Cerr << "Error in computeDSTIEN()"
           << ": all response samples must have the same size (" << num_responses
           << ") as the first sample"
           << std::endl;
      abort_handler(-1);
    }
  }

  if ((0. <= coverage) && (coverage <= 1.)) {
    // Ok
  }
  else {
    Cerr << "Error in computeDSTIEN()"
         << ": coverage (" << coverage
         << ") must belong to the closed interval [0,1]"
         << std::endl;
    abort_handler(-1);
  }

  if ((0. <= alpha) && (alpha <= 1.)) {
    // Ok
  }
  else {
    Cerr << "Error in computeDSTIEN()"
         << ": alpha (" << alpha
         << ") must belong to the closed interval [0,1]"
         << std::endl;
    abort_handler(-1);
  }

  if (dstien_mus.length() != num_responses) {
    dstien_mus.resize(num_responses);
  }

  if (sample_sigmas.length() != num_responses) {
    sample_sigmas.resize(num_responses);
  }

  if (dstien_sigmas.length() != num_responses) {
    dstien_sigmas.resize(num_responses);
  }

  // Initialize the output variables
  num_valid_samples = 0;
  dstien_mus        = 0.;
  delta_mf          = 0.;
  sample_sigmas     = 0.;
  dstien_sigmas     = 0.;

  // Determine the amount of valid samples
  std::vector<bool> sample_valid_status(num_samples,false);
  {
    IntRespMCIter it = resp_samples.begin();
    for (size_t j = 0; j < num_samples; ++j, ++it) {
      bool sample_is_valid = true;
      for (size_t k = 0; (k < num_responses) && sample_is_valid; ++k) {
        sample_is_valid = std::isfinite(it->second.function_value(k));
      } // for k
      if (sample_is_valid) {
        num_valid_samples += 1;
        sample_valid_status[j] = true;
      }
    } // for j
  }

  if (num_valid_samples == 0) {
    dstien_mus    = std::numeric_limits<Real>::quiet_NaN();
    delta_mf      = std::numeric_limits<Real>::quiet_NaN();
    sample_sigmas = std::numeric_limits<Real>::quiet_NaN();
    dstien_sigmas = std::numeric_limits<Real>::quiet_NaN();
  }
  else {
    Real m = static_cast<Real>(num_valid_samples);

    // Compute DSTIEN mus
    {
      IntRespMCIter it = resp_samples.begin();
      for (size_t j = 0; j < num_samples; ++j, ++it) {
        if (sample_valid_status[j]) {
          for (size_t k = 0; k < num_responses; ++k) {
            dstien_mus[k] += it->second.function_value(k);
          } // for k
        }
      } // for j
    }

    dstien_mus *= 1./m;

    // Compute DSTIEN sigmas
    if (num_valid_samples == 1) {
      delta_mf      = std::numeric_limits<Real>::quiet_NaN();
      sample_sigmas = std::numeric_limits<Real>::quiet_NaN();
      dstien_sigmas = std::numeric_limits<Real>::quiet_NaN();
    }
    else {
      {
        IntRespMCIter it = resp_samples.begin();
        for (size_t j = 0; j < num_samples; ++j, ++it) {
          if (sample_valid_status[j]) {
            for (size_t k = 0; k < num_responses; ++k) {
              Real diff = it->second.function_value(k) - dstien_mus[k];
              sample_sigmas[k] += diff * diff;
            } // for k
          }
        } // for j
      }

      Real conversionFactor = computeDSTIEN_conversion_factor( num_valid_samples
                                                             , alpha
                                                             );

      delta_mf = std_normal_coverage_inverse(coverage) * conversionFactor;

      sample_sigmas *= 1./(m - 1.);
      for (size_t k = 0; k < num_responses; ++k) {
        sample_sigmas[k] = std::sqrt( sample_sigmas[k] );
      } // for k

      dstien_sigmas =  sample_sigmas;
      dstien_sigmas *= conversionFactor;
    } // if (num_valid_samples == 1) else
  } // if (num_valid_samples == 0) else
} // void computeDSTIEN()

} // namespace Dakota
