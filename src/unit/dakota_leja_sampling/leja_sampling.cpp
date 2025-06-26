/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "BasisPolynomial.hpp"
#include "DakotaResponse.hpp"
#include "MarginalsCorrDistribution.hpp"
#include "dakota_global_defs.hpp"
// #include "SharedPolyApproxData.hpp"
#include <gtest/gtest.h>

#include <cassert>
#include <iostream>

#include "SharedOrthogPolyApproxData.hpp"
#include "nested_sampling.hpp"

// Using Boost distributions for cross-platform stability
// Using Boost MT since using Boost distributions

// Portability for deprecated Boost integer_log2.hpp header used in
// Boost 1.69 random library. To be removed once we migrate to std
// library RNG.
#include <boost/version.hpp>
#if (BOOST_VERSION < 107000) && !defined(BOOST_ALLOW_DEPRECATED_HEADERS)
// could alternately use: #define BOOST_PENDING_INTEGER_LOG2_HPP 1
#define BOOST_ALLOW_DEPRECATED_HEADERS 1
#include <boost/random/binomial_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/negative_binomial_distribution.hpp>
#include <boost/random/poisson_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#undef BOOST_ALLOW_DEPRECATED_HEADERS
#else
#include <boost/random/binomial_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/negative_binomial_distribution.hpp>
#include <boost/random/poisson_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#endif

namespace Dakota {
namespace TestLejaSampling {

/** \brief Create a MultivariateDistribution object such that all
 * variables are uniform in [l_bnd, u_bnd]

  Note this function sets specific distribution parameters
  that are not general. They must coincide with the parameters used
  in generate_samples

 */
Pecos::MultivariateDistribution
initialize_homogeneous_uniform_aleatory_dist_params(short utype, int num_vars) {
  Pecos::MultivariateDistribution mvd(Pecos::MARGINALS_CORRELATIONS);
  auto mvd_rep = std::static_pointer_cast<Pecos::MarginalsCorrDistribution>(
      mvd.multivar_dist_rep());

  ShortArray rv_types(num_vars, utype);
  mvd_rep->initialize_types(rv_types);  // default active_vars

  RealSymMatrix uv_corr;
  mvd_rep->initialize_correlations(uv_corr);  // default active_corr

  switch (utype) {
    case Pecos::STD_UNIFORM: {
      RealArray uuv_l_bnds(num_vars, -1.), uuv_u_bnds(num_vars, 1.);
      mvd_rep->push_parameters(utype, Pecos::U_LWR_BND, uuv_l_bnds);
      mvd_rep->push_parameters(utype, Pecos::U_UPR_BND, uuv_u_bnds);
      break;
    }
    case Pecos::POISSON: {
      RealArray puv_lambdas(num_vars, 100.);
      mvd_rep->push_parameters(utype, Pecos::P_LAMBDA, puv_lambdas);
      break;
    }
    case Pecos::BINOMIAL: {
      RealArray biuv_p_per_tr(num_vars, 0.5);
      UIntArray biuv_num_trials(num_vars, 20);
      mvd_rep->push_parameters(utype, Pecos::BI_P_PER_TRIAL, biuv_p_per_tr);
      mvd_rep->push_parameters(utype, Pecos::BI_TRIALS, biuv_num_trials);
      break;
    }
    case Pecos::NEGATIVE_BINOMIAL: {
      RealArray nbuv_p_per_tr(num_vars, 0.5);
      UIntArray nbuv_num_trials(num_vars, 20);
      mvd_rep->push_parameters(utype, Pecos::NBI_P_PER_TRIAL, nbuv_p_per_tr);
      mvd_rep->push_parameters(utype, Pecos::NBI_TRIALS, nbuv_num_trials);
      break;
    }
  }

  return mvd;
}

template <typename T>
void generate_samples(T &rvt, int num_vars, int num_samples,
                      RealMatrix &samples) {
  samples.shapeUninitialized(num_vars, num_samples);
  for (int i = 0; i < num_vars; i++)
    for (int j = 0; j < num_samples; j++) samples(i, j) = rvt();
}

/**  Note this function sets specific distribution parameters
     that are not general. They must coincide with the parameters used
     in initialize_homogeneous_uniform_aleatory_dist_params
*/
void generate_samples(short utype, int num_vars, int num_candidate_samples,
                      int seed, RealMatrix &candidate_samples) {
  boost::mt19937 gen(seed);

  switch (utype) {
    case Pecos::STD_UNIFORM: {
      boost::random::uniform_real_distribution<Real> un_dist(-1., 1.);
      boost::variate_generator<boost::mt19937,
                               boost::random::uniform_real_distribution<Real> >
          un_rvt(gen, un_dist);
      generate_samples(un_rvt, num_vars, num_candidate_samples,
                       candidate_samples);
      break;
    }
    case Pecos::POISSON: {
      Real poisson_lambda = 100.;
      boost::poisson_distribution<> po_dist(poisson_lambda);
      boost::variate_generator<boost::mt19937, boost::poisson_distribution<> >
          po_rvt(gen, po_dist);
      generate_samples(po_rvt, num_vars, num_candidate_samples,
                       candidate_samples);
      break;
    }
    case Pecos::BINOMIAL: {
      int n = 20;
      Real p = 0.5;
      boost::random::binomial_distribution<> bi_dist(n, p);
      boost::variate_generator<boost::mt19937,
                               boost::random::binomial_distribution<> >
          bi_rvt(gen, bi_dist);
      generate_samples(bi_rvt, num_vars, num_candidate_samples,
                       candidate_samples);
      break;
    }
    case Pecos::NEGATIVE_BINOMIAL: {
      int n = 20;
      Real p = 0.5;
      boost::random::negative_binomial_distribution<> nbi_dist(n, p);
      boost::variate_generator<boost::mt19937,
                               boost::random::negative_binomial_distribution<> >
          nbi_rvt(gen, nbi_dist);
      generate_samples(nbi_rvt, num_vars, num_candidate_samples,
                       candidate_samples);
    } break;
  }
}

void test_leja_sequence_helper(short utype, int num_vars,
                               int num_initial_samples, int num_new_samples,
                               const RealMatrix &candidate_samples) {
  // Currently Leja Sampler assumes that the samples are in U space
  // that is the native space of the polynomial. For Legendre polynomials
  // the parameter range is [-1,1]
  Pecos::MultivariateDistribution mvd =
      initialize_homogeneous_uniform_aleatory_dist_params(utype, num_vars);

  // Build polynomial basis using default basis configuration options
  Pecos::BasisConfigOptions bc_options;
  std::vector<Pecos::BasisPolynomial> poly_basis;
  ShortArray basis_types, colloc_rules;
  Pecos::SharedOrthogPolyApproxData::construct_basis(
      mvd, bc_options, poly_basis, basis_types, colloc_rules);

  Pecos::SharedPolyApproxData::update_basis_distribution_parameters(mvd,
                                                                    poly_basis);

  // Initialize sampling object
  LejaSampler sampler;
  sampler.set_precondition(true);
  sampler.set_polynomial_basis(poly_basis);

  // Generate initial samples
  RealMatrix initial_samples, empty_matrix;
  // The test only works if we use the same degree at every step.
  int num_total_samples = num_initial_samples + num_new_samples;
  sampler.set_total_degree_basis_from_num_samples(num_vars, num_total_samples);
  // First time sampler is called there are no samples we wish to reuse
  // so pass in empty matrix
  sampler.Sampler::enrich_samples(num_vars, empty_matrix, num_initial_samples,
                                  candidate_samples, initial_samples);

  // Enrich initial_samples
  RealMatrix enriched_samples;
  sampler.set_total_degree_basis_from_num_samples(num_vars, num_total_samples);
  sampler.Sampler::enrich_samples(num_vars, initial_samples, num_new_samples,
                                  candidate_samples, enriched_samples);

  RealMatrix enriched_samples_subset(Teuchos::Copy, enriched_samples, num_vars,
                                     num_initial_samples);
  enriched_samples_subset -= initial_samples;
  EXPECT_TRUE((enriched_samples_subset.normInf() <
               10. * std::numeric_limits<double>::epsilon()));

  // Run sampler and generate all points in one go
  RealMatrix combined_samples;
  sampler.set_total_degree_basis_from_num_samples(num_vars, num_total_samples);
  sampler.Sampler::enrich_samples(num_vars, empty_matrix, num_total_samples,
                                  candidate_samples, combined_samples);

  // Check samples obtained by running algorithm once are the same
  // as those obtained running the algorithm in steps.
  // write_data(std::cout, combined_samples);
  // write_data(std::cout, enriched_samples);
  combined_samples -= enriched_samples;
  // std::cout << "difference:\n" << std::endl;
  // write_data(std::cout, combined_samples);
  // std::cout << "normInf difference:" <<
  // combined_samples.normInf()<<std::endl;
  EXPECT_TRUE((combined_samples.normInf() <
               10. * std::numeric_limits<double>::epsilon()));
}

void test_uniform_leja_sequence() {
  int num_vars = 2;
  int num_initial_samples = 10;
  int num_new_samples = 20;
  int num_candidate_samples = 10000;
  int seed = 1;

  // NEGATIVE_BINOMIAL fails on RHEL7, BINOMIAL and POISSON give inf basis
  // matrices on Windows
  std::vector<short> utypes = {Pecos::STD_UNIFORM, Pecos::POISSON};
  RealMatrix candidate_samples;

  for (auto ut : utypes) {
    generate_samples(ut, num_vars, num_candidate_samples, seed,
                     candidate_samples);

    test_leja_sequence_helper(ut, num_vars, num_initial_samples,
                              num_new_samples, candidate_samples);
  }
}

}  // namespace TestLejaSampling
}  // end namespace Dakota

//____________________________________________________________________________//

TEST(leja_sampling_tests, all_tests) {
  using namespace Dakota::TestLejaSampling;

  test_uniform_leja_sequence();

  SUCCEED();
}

//____________________________________________________________________________//

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
